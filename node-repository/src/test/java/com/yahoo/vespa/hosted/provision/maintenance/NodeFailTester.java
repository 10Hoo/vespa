package com.yahoo.vespa.hosted.provision.maintenance;

import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.ApplicationName;
import com.yahoo.config.provision.Capacity;
import com.yahoo.config.provision.ClusterSpec;
import com.yahoo.config.provision.Environment;
import com.yahoo.config.provision.HostLivenessTracker;
import com.yahoo.config.provision.HostSpec;
import com.yahoo.config.provision.InstanceName;
import com.yahoo.config.provision.NodeType;
import com.yahoo.config.provision.RegionName;
import com.yahoo.config.provision.TenantName;
import com.yahoo.config.provision.Zone;
import com.yahoo.test.ManualClock;
import com.yahoo.transaction.NestedTransaction;
import com.yahoo.vespa.applicationmodel.ApplicationInstance;
import com.yahoo.vespa.applicationmodel.ApplicationInstanceId;
import com.yahoo.vespa.applicationmodel.ApplicationInstanceReference;
import com.yahoo.vespa.applicationmodel.ClusterId;
import com.yahoo.vespa.applicationmodel.ConfigId;
import com.yahoo.vespa.applicationmodel.HostName;
import com.yahoo.vespa.applicationmodel.ServiceCluster;
import com.yahoo.vespa.applicationmodel.ServiceInstance;
import com.yahoo.vespa.applicationmodel.ServiceType;
import com.yahoo.vespa.applicationmodel.TenantId;
import com.yahoo.vespa.curator.Curator;
import com.yahoo.vespa.curator.mock.MockCurator;
import com.yahoo.vespa.curator.transaction.CuratorTransaction;
import com.yahoo.vespa.hosted.provision.Node;
import com.yahoo.vespa.hosted.provision.NodeRepository;
import com.yahoo.vespa.hosted.provision.node.Flavor;
import com.yahoo.vespa.hosted.provision.node.NodeFlavors;
import com.yahoo.vespa.hosted.provision.provisioning.NodeRepositoryProvisioner;
import com.yahoo.vespa.hosted.provision.testutils.FlavorConfigBuilder;
import com.yahoo.vespa.orchestrator.ApplicationIdNotFoundException;
import com.yahoo.vespa.orchestrator.ApplicationStateChangeDeniedException;
import com.yahoo.vespa.orchestrator.BatchHostNameNotFoundException;
import com.yahoo.vespa.orchestrator.BatchInternalErrorException;
import com.yahoo.vespa.orchestrator.HostNameNotFoundException;
import com.yahoo.vespa.orchestrator.Orchestrator;
import com.yahoo.vespa.orchestrator.policy.BatchHostStateChangeDeniedException;
import com.yahoo.vespa.orchestrator.policy.HostStateChangeDeniedException;
import com.yahoo.vespa.orchestrator.status.ApplicationInstanceStatus;
import com.yahoo.vespa.orchestrator.status.HostStatus;
import com.yahoo.vespa.service.monitor.ServiceMonitor;
import com.yahoo.vespa.service.monitor.ServiceMonitorStatus;

import java.time.Clock;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;

import static org.junit.Assert.assertEquals;

/**
 * @author bratseth
 */
public class NodeFailTester {

    // Immutable components
    public static final ApplicationId app1 = ApplicationId.from(TenantName.from("foo1"), ApplicationName.from("bar"), InstanceName.from("fuz"));
    public static final ApplicationId app2 = ApplicationId.from(TenantName.from("foo2"), ApplicationName.from("bar"), InstanceName.from("fuz"));
    public static final NodeFlavors nodeFlavors = FlavorConfigBuilder.createDummies("default", "docker");
    private static final Zone zone = new Zone(Environment.prod, RegionName.from("us-east"));
    private static final Duration downtimeLimitOneHour = Duration.ofMinutes(60);

    // Components with state
    public final ManualClock clock;
    public final NodeRepository nodeRepository;
    public NodeFailer failer;
    public ServiceMonitorStub serviceMonitor;
    public MockDeployer deployer;
    private final TestHostLivenessTracker hostLivenessTracker;
    private final Orchestrator orchestrator;
    private final NodeRepositoryProvisioner provisioner;
    private final Curator curator;

    public NodeFailTester() {
        clock = new ManualClock();
        curator = new MockCurator();
        nodeRepository = new NodeRepository(nodeFlavors, curator, clock);
        provisioner = new NodeRepositoryProvisioner(nodeRepository, nodeFlavors, zone);
        hostLivenessTracker = new TestHostLivenessTracker(clock);
        orchestrator = new OrchestratorMock();
    }
    
    public static NodeFailTester withTwoApplications() {
        NodeFailTester tester = new NodeFailTester();
        
        tester.createReadyNodes(16);
        tester.createHostNodes(3);

        // Create applications
        ClusterSpec clusterApp1 = ClusterSpec.request(ClusterSpec.Type.container, ClusterSpec.Id.from("test"), Optional.empty());
        ClusterSpec clusterApp2 = ClusterSpec.request(ClusterSpec.Type.content, ClusterSpec.Id.from("test"), Optional.empty());
        int wantedNodesApp1 = 5;
        int wantedNodesApp2 = 7;
        tester.activate(app1, clusterApp1, wantedNodesApp1);
        tester.activate(app2, clusterApp2, wantedNodesApp2);
        assertEquals(wantedNodesApp1, tester.nodeRepository.getNodes(app1, Node.State.active).size());
        assertEquals(wantedNodesApp2, tester.nodeRepository.getNodes(app2, Node.State.active).size());

        Map<ApplicationId, MockDeployer.ApplicationContext> apps = new HashMap<>();
        apps.put(app1, new MockDeployer.ApplicationContext(app1, clusterApp1, wantedNodesApp1, Optional.of("default"), 1));
        apps.put(app2, new MockDeployer.ApplicationContext(app2, clusterApp2, wantedNodesApp2, Optional.of("default"), 1));
        tester.deployer = new MockDeployer(tester.provisioner, apps);
        tester.serviceMonitor = new ServiceMonitorStub(apps, tester.nodeRepository);
        tester.failer = tester.createFailer();
        return tester;
    }
    
    public void suspend(ApplicationId app) {
        try {
            orchestrator.suspend(app);
        }
        catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public NodeFailer createFailer() {
        return new NodeFailer(deployer, hostLivenessTracker, serviceMonitor, nodeRepository, downtimeLimitOneHour, clock, orchestrator);
    }

    public void allNodesMakeAConfigRequestExcept(Node ... deadNodeArray) {
        Set<Node> deadNodes = new HashSet<>(Arrays.asList(deadNodeArray));
        for (Node node : nodeRepository.getNodes(NodeType.tenant)) {
            if ( ! deadNodes.contains(node) && node.flavor().getType() != Flavor.Type.DOCKER_CONTAINER)
                hostLivenessTracker.receivedRequestFrom(node.hostname());
        }
    }

    public void createReadyNodes(int count) {
        createReadyNodes(count, 0);
    }

    public void createReadyNodes(int count, int startIndex) {
        createReadyNodes(count, startIndex, "default");
    }

    public void createReadyNodes(int count, int startIndex, String flavor) {
        createReadyNodes(count, startIndex, nodeFlavors.getFlavorOrThrow(flavor), NodeType.tenant);
    }

    private void createReadyNodes(int count, int startIndex, Flavor flavor, NodeType nodeType) {
        List<Node> nodes = new ArrayList<>(count);
        for (int i = startIndex; i < startIndex + count; i++)
            nodes.add(nodeRepository.createNode("node" + i, "host" + i, Optional.empty(), flavor, nodeType));
        nodes = nodeRepository.addNodes(nodes);
        nodeRepository.setReady(nodes);
    }

    private void createHostNodes(int count) {
        List<Node> nodes = new ArrayList<>(count);
        for (int i = 0; i < count; i++)
            nodes.add(nodeRepository.createNode("parent" + i, "parent" + i, Optional.empty(), nodeFlavors.getFlavorOrThrow("default"), NodeType.host));
        nodes = nodeRepository.addNodes(nodes);
        nodeRepository.setReady(nodes);
    }

    private void activate(ApplicationId applicationId, ClusterSpec cluster, int nodeCount) {
        List<HostSpec> hosts = provisioner.prepare(applicationId, cluster, Capacity.fromNodeCount(nodeCount), 1, null);
        NestedTransaction transaction = new NestedTransaction().add(new CuratorTransaction(curator));
        provisioner.activate(transaction, applicationId, hosts);
        transaction.commit();
    }

    /** Returns the node with the highest membership index from the given set of allocated nodes */
    public Node highestIndex(List<Node> nodes) {
        Node highestIndex = null;
        for (Node node : nodes) {
            if (highestIndex == null || node.allocation().get().membership().index() >
                                        highestIndex.allocation().get().membership().index())
                highestIndex = node;
        }
        return highestIndex;
    }

    /** This is a fully functional implementation */
    private static class TestHostLivenessTracker implements HostLivenessTracker {

        private final Clock clock;
        private final Map<String, Instant> lastRequestFromHost = new HashMap<>();

        public TestHostLivenessTracker(Clock clock) {
            this.clock = clock;
        }

        @Override
        public void receivedRequestFrom(String hostname) {
            lastRequestFromHost.put(hostname, clock.instant());
        }

        @Override
        public Optional<Instant> lastRequestFrom(String hostname) {
            return Optional.ofNullable(lastRequestFromHost.get(hostname));
        }

    }

    public static class ServiceMonitorStub implements ServiceMonitor {

        private final Map<ApplicationId, MockDeployer.ApplicationContext> apps;
        private final NodeRepository nodeRepository;

        private Set<String> downHosts = new HashSet<>();
        private boolean statusIsKnown = true;

        /** Create a service monitor where all nodes are initially up */
        public ServiceMonitorStub(Map<ApplicationId, MockDeployer.ApplicationContext> apps, NodeRepository nodeRepository) {
            this.apps = apps;
            this.nodeRepository = nodeRepository;
        }

        public void setHostDown(String hostname) {
            downHosts.add(hostname);
        }

        public void setHostUp(String hostname) {
            downHosts.remove(hostname);
        }

        public void setStatusIsKnown(boolean statusIsKnown) {
            this.statusIsKnown = statusIsKnown;
        }

        private ServiceMonitorStatus getHostStatus(String hostname) {
            if ( ! statusIsKnown) return ServiceMonitorStatus.NOT_CHECKED;
            if (downHosts.contains(hostname)) return ServiceMonitorStatus.DOWN;
            return ServiceMonitorStatus.UP;
        }

        @Override
        public Map<ApplicationInstanceReference, ApplicationInstance<ServiceMonitorStatus>> queryStatusOfAllApplicationInstances() {
            // Convert apps information to the response payload to return
            Map<ApplicationInstanceReference, ApplicationInstance<ServiceMonitorStatus>> status = new HashMap<>();
            for (Map.Entry<ApplicationId, MockDeployer.ApplicationContext> app : apps.entrySet()) {
                Set<ServiceInstance<ServiceMonitorStatus>> serviceInstances = new HashSet<>();
                for (Node node : nodeRepository.getNodes(app.getValue().id(), Node.State.active)) {
                    serviceInstances.add(new ServiceInstance<>(new ConfigId("configid"),
                                                               new HostName(node.hostname()),
                                                               getHostStatus(node.hostname())));
                }
                Set<ServiceCluster<ServiceMonitorStatus>> serviceClusters = new HashSet<>();
                serviceClusters.add(new ServiceCluster<>(new ClusterId(app.getValue().cluster().id().value()),
                                                         new ServiceType("serviceType"),
                                                         serviceInstances));
                TenantId tenantId = new TenantId(app.getKey().tenant().value());
                ApplicationInstanceId applicationInstanceId = new ApplicationInstanceId(app.getKey().application().value());
                status.put(new ApplicationInstanceReference(tenantId, applicationInstanceId),
                           new ApplicationInstance<>(tenantId, applicationInstanceId, serviceClusters));
            }
            return status;
        }

    }

    class OrchestratorMock implements Orchestrator {

        Set<ApplicationId> suspendedApplications = new HashSet<>();

        @Override
        public HostStatus getNodeStatus(HostName hostName) throws HostNameNotFoundException {
            return null;
        }

        @Override
        public void resume(HostName hostName) throws HostStateChangeDeniedException, HostNameNotFoundException {}

        @Override
        public void suspend(HostName hostName) throws HostStateChangeDeniedException, HostNameNotFoundException {}

        @Override
        public ApplicationInstanceStatus getApplicationInstanceStatus(ApplicationId appId) throws ApplicationIdNotFoundException {
            return suspendedApplications.contains(appId)
                   ? ApplicationInstanceStatus.ALLOWED_TO_BE_DOWN : ApplicationInstanceStatus.NO_REMARKS;
        }

        @Override
        public Set<ApplicationId> getAllSuspendedApplications() {
            return null;
        }

        @Override
        public void resume(ApplicationId appId) throws ApplicationStateChangeDeniedException, ApplicationIdNotFoundException {
            suspendedApplications.remove(appId);
        }

        @Override
        public void suspend(ApplicationId appId) throws ApplicationStateChangeDeniedException, ApplicationIdNotFoundException {
            suspendedApplications.add(appId);
        }

        @Override
        public void suspendAll(HostName parentHostname, List<HostName> hostNames) throws BatchInternalErrorException, BatchHostStateChangeDeniedException, BatchHostNameNotFoundException {
            throw new RuntimeException("Not implemented");
        }
    }

}
