// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.provision.maintenance;

import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.ApplicationName;
import com.yahoo.config.provision.Capacity;
import com.yahoo.config.provision.ClusterSpec;
import com.yahoo.config.provision.Environment;
import com.yahoo.config.provision.HostSpec;
import com.yahoo.config.provision.InstanceName;
import com.yahoo.config.provision.RegionName;
import com.yahoo.config.provision.TenantName;
import com.yahoo.config.provision.Zone;
import com.yahoo.test.ManualClock;
import com.yahoo.transaction.NestedTransaction;
import com.yahoo.vespa.curator.Curator;
import com.yahoo.vespa.curator.mock.MockCurator;
import com.yahoo.vespa.hosted.provision.Node;
import com.yahoo.vespa.hosted.provision.NodeList;
import com.yahoo.vespa.hosted.provision.NodeRepository;
import com.yahoo.vespa.hosted.provision.node.Configuration;
import com.yahoo.vespa.hosted.provision.node.NodeFlavors;
import com.yahoo.vespa.hosted.provision.provisioning.NodeRepositoryProvisioner;
import com.yahoo.vespa.curator.transaction.CuratorTransaction;
import com.yahoo.vespa.hosted.provision.testutils.FlavorConfigBuilder;
import org.junit.Test;

import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import static org.junit.Assert.assertEquals;

/**
 * @author bratseth
 */
public class ApplicationMaintainerTest {

    private Curator curator = new MockCurator();

    @Test
    public void test_application_maintenance() throws InterruptedException {
        ManualClock clock = new ManualClock();
        Zone zone = new Zone(Environment.prod, RegionName.from("us-east"));
        NodeFlavors nodeFlavors = FlavorConfigBuilder.createDummies("default");
        NodeRepository nodeRepository = new NodeRepository(nodeFlavors, curator, clock);

        createReadyNodes(15, nodeRepository, nodeFlavors);
        createHostNodes(2, nodeRepository, nodeFlavors);

        Fixture fixture = new Fixture(zone, nodeRepository, nodeFlavors);

        // Create applications
        fixture.activate();

        // Fail and park some nodes
        nodeRepository.fail(nodeRepository.getNodes(fixture.app1).get(3).hostname());
        nodeRepository.fail(nodeRepository.getNodes(fixture.app2).get(0).hostname());
        nodeRepository.park(nodeRepository.getNodes(fixture.app2).get(4).hostname());
        int failedInApp1 = 1;
        int failedOrParkedInApp2 = 2;
        assertEquals(fixture.wantedNodesApp1 - failedInApp1, nodeRepository.getNodes(fixture.app1, Node.State.active).size());
        assertEquals(fixture.wantedNodesApp2 - failedOrParkedInApp2, nodeRepository.getNodes(fixture.app2, Node.State.active).size());
        assertEquals(failedInApp1 + failedOrParkedInApp2, nodeRepository.getNodes(Node.Type.tenant, Node.State.failed, Node.State.parked).size());
        assertEquals(3, nodeRepository.getNodes(Node.Type.tenant, Node.State.ready).size());
        assertEquals(2, nodeRepository.getNodes(Node.Type.host, Node.State.ready).size());

        // Cause maintenance deployment which will allocate replacement nodes
        fixture.runApplicationMaintainer();
        assertEquals(fixture.wantedNodesApp1, nodeRepository.getNodes(fixture.app1, Node.State.active).size());
        assertEquals(fixture.wantedNodesApp2, nodeRepository.getNodes(fixture.app2, Node.State.active).size());
        assertEquals(0, nodeRepository.getNodes(Node.Type.tenant, Node.State.ready).size());

        // Reactivate the previously failed nodes
        nodeRepository.reactivate(nodeRepository.getNodes(Node.Type.tenant, Node.State.failed).get(0).hostname());
        nodeRepository.reactivate(nodeRepository.getNodes(Node.Type.tenant, Node.State.failed).get(0).hostname());
        nodeRepository.reactivate(nodeRepository.getNodes(Node.Type.tenant, Node.State.parked).get(0).hostname());
        int reactivatedInApp1 = 1;
        int reactivatedInApp2 = 2;
        assertEquals(0, nodeRepository.getNodes(Node.Type.tenant, Node.State.failed).size());
        assertEquals(fixture.wantedNodesApp1 + reactivatedInApp1, nodeRepository.getNodes(fixture.app1, Node.State.active).size());
        assertEquals(fixture.wantedNodesApp2 + reactivatedInApp2, nodeRepository.getNodes(fixture.app2, Node.State.active).size());
        assertEquals("The reactivated nodes are now active but not part of the application",
                     0, fixture.getNodes(Node.State.active).retired().size());

        // Cause maintenance deployment which will update the applications with the re-activated nodes
        fixture.runApplicationMaintainer();
        assertEquals("Superflous content nodes are retired",
                     reactivatedInApp2, fixture.getNodes(Node.State.active).retired().size());
        assertEquals("Superflous container nodes are deactivated (this makes little point for container nodes)",
                     reactivatedInApp1, fixture.getNodes(Node.State.inactive).size());
    }

    private void createReadyNodes(int count, NodeRepository nodeRepository, NodeFlavors nodeFlavors) {
        List<Node> nodes = new ArrayList<>(count);
        for (int i = 0; i < count; i++)
            nodes.add(nodeRepository.createNode("node" + i, "host" + i, Optional.empty(), new Configuration(nodeFlavors.getFlavorOrThrow("default")), Node.Type.tenant));
        nodes = nodeRepository.addNodes(nodes);
        nodeRepository.setReady(nodes);
    }

    private void createHostNodes(int count, NodeRepository nodeRepository, NodeFlavors nodeFlavors) {
        List<Node> nodes = new ArrayList<>(count);
        for (int i = 0; i < count; i++)
            nodes.add(nodeRepository.createNode("hostNode" + i, "realHost" + i, Optional.empty(), new Configuration(nodeFlavors.getFlavorOrThrow("default")), Node.Type.host));
        nodes = nodeRepository.addNodes(nodes);
        nodeRepository.setReady(nodes);
    }

    private class Fixture {

        final NodeRepository nodeRepository;
        final NodeRepositoryProvisioner provisioner;

        final ApplicationId app1 = ApplicationId.from(TenantName.from("foo1"), ApplicationName.from("bar"), InstanceName.from("fuz"));
        final ApplicationId app2 = ApplicationId.from(TenantName.from("foo2"), ApplicationName.from("bar"), InstanceName.from("fuz"));
        final ClusterSpec clusterApp1 = ClusterSpec.from(ClusterSpec.Type.container, ClusterSpec.Id.from("test"), Optional.empty());
        final ClusterSpec clusterApp2 = ClusterSpec.from(ClusterSpec.Type.content, ClusterSpec.Id.from("test"), Optional.empty());
        final int wantedNodesApp1 = 5;
        final int wantedNodesApp2 = 7;

        Fixture(Zone zone, NodeRepository nodeRepository, NodeFlavors flavors) {
            this.nodeRepository = nodeRepository;
            this.provisioner =  new NodeRepositoryProvisioner(nodeRepository, flavors, zone);
        }

        void activate() {
            activate(app1, clusterApp1, wantedNodesApp1, provisioner);
            activate(app2, clusterApp2, wantedNodesApp2, provisioner);
            assertEquals(wantedNodesApp1, nodeRepository.getNodes(app1, Node.State.active).size());
            assertEquals(wantedNodesApp2, nodeRepository.getNodes(app2, Node.State.active).size());
        }

        private void activate(ApplicationId applicationId, ClusterSpec cluster, int nodeCount, NodeRepositoryProvisioner provisioner) {
            List<HostSpec> hosts = provisioner.prepare(applicationId, cluster, Capacity.fromNodeCount(nodeCount), 1, null);
            NestedTransaction transaction = new NestedTransaction().add(new CuratorTransaction(curator));
            provisioner.activate(transaction, applicationId, hosts);
            transaction.commit();
        }

        void runApplicationMaintainer() {
            Map<ApplicationId, MockDeployer.ApplicationContext> apps = new HashMap<>();
            apps.put(app1, new MockDeployer.ApplicationContext(app1, clusterApp1, wantedNodesApp1, Optional.of("default"), 1));
            apps.put(app2, new MockDeployer.ApplicationContext(app2, clusterApp2, wantedNodesApp2, Optional.of("default"), 1));
            MockDeployer deployer = new MockDeployer(provisioner, apps);
            new ApplicationMaintainer(deployer, nodeRepository, Duration.ofMinutes(30)).run();
        }

        NodeList getNodes(Node.State ... states) {
            return new NodeList(nodeRepository.getNodes(Node.Type.tenant, states));
        }

    }

}
