// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.provision.provisioning;

import com.yahoo.component.Version;
import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.ClusterSpec;
import com.yahoo.config.provision.HostName;
import com.yahoo.config.provision.HostSpec;
import com.yahoo.config.provision.Zone;
import com.yahoo.transaction.NestedTransaction;
import com.yahoo.vespa.hosted.provision.Node;
import com.yahoo.vespa.hosted.provision.lb.LoadBalancer;
import com.yahoo.vespa.hosted.provision.lb.LoadBalancerId;
import com.yahoo.vespa.hosted.provision.lb.LoadBalancerService;
import com.yahoo.vespa.hosted.provision.lb.Real;
import com.yahoo.vespa.hosted.provision.node.Agent;
import org.junit.Before;
import org.junit.Test;

import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * @author mpolden
 */
public class LoadBalancerProvisionerTest {

    private final ApplicationId app1 = ApplicationId.from("tenant1", "application1", "default");
    private final ApplicationId app2 = ApplicationId.from("tenant2", "application2", "default");

    private ProvisioningTester tester;
    private LoadBalancerService service;
    private LoadBalancerProvisioner loadBalancerProvisioner;

    @Before
    public void before() {
        tester = new ProvisioningTester(Zone.defaultZone());
        service = tester.loadBalancerService();
        loadBalancerProvisioner = new LoadBalancerProvisioner(tester.nodeRepository(), service);
    }

    @Test
    public void provision_load_balancer() {
        ClusterSpec.Id containerCluster1 = ClusterSpec.Id.from("qrs1");
        ClusterSpec.Id contentCluster = ClusterSpec.Id.from("content");
        tester.activate(app1, prepare(app1,
                                      clusterRequest(ClusterSpec.Type.container, containerCluster1),
                                      clusterRequest(ClusterSpec.Type.content, contentCluster)));
        tester.activate(app2, prepare(app2,
                                      clusterRequest(ClusterSpec.Type.container, ClusterSpec.Id.from("qrs"))));

        // Provision a load balancer for each application
        Map<LoadBalancerId, LoadBalancer> loadBalancers = loadBalancerProvisioner.provision(app1);
        loadBalancerProvisioner.provision(app2);
        assertEquals(1, loadBalancers.size());

        LoadBalancer loadBalancer = loadBalancers.values().iterator().next();
        assertEquals(loadBalancer.id().application(), app1);
        assertEquals(loadBalancer.id().cluster(), containerCluster1);
        assertEquals(loadBalancer.ports(), Collections.singletonList(4443));
        assertEquals(loadBalancer.reals().get(0).ipAddress(), "127.0.0.1");
        assertEquals(loadBalancer.reals().get(0).port(), 4443);
        assertEquals(loadBalancer.reals().get(1).ipAddress(), "127.0.0.2");
        assertEquals(loadBalancer.reals().get(1).port(), 4443);

        // A container is failed
        List<Node> containers = tester.getNodes(app1).type(ClusterSpec.Type.container).asList();
        Node container1 = containers.get(0);
        Node container2 = containers.get(1);
        tester.nodeRepository().fail(container1.hostname(), Agent.system, "Failed by unit test");

        // Reprovisioning load balancer removes failed container
        loadBalancer = loadBalancerProvisioner.provision(app1).values().iterator().next();
        assertEquals(1, loadBalancer.reals().size());
        assertEquals(container2.hostname(), loadBalancer.reals().get(0).hostname().value());

        // Redeploying replaces failed node
        tester.activate(app1, prepare(app1,
                                      clusterRequest(ClusterSpec.Type.container, containerCluster1),
                                      clusterRequest(ClusterSpec.Type.content, contentCluster)));

        // Reprovisioning load balancer adds the new node
        Node container3 = tester.getNodes(app1).type(ClusterSpec.Type.container).asList().get(1);
        loadBalancer = loadBalancerProvisioner.provision(app1).values().iterator().next();
        assertEquals(2, loadBalancer.reals().size());
        assertEquals(container3.hostname(), loadBalancer.reals().get(1).hostname().value());

        // Add another container cluster
        ClusterSpec.Id containerCluster2 = ClusterSpec.Id.from("qrs2");
        tester.activate(app1, prepare(app1,
                                      clusterRequest(ClusterSpec.Type.container, containerCluster1),
                                      clusterRequest(ClusterSpec.Type.container, containerCluster2),
                                      clusterRequest(ClusterSpec.Type.content, contentCluster)));

        // Load balancer is provisioned for second container cluster
        loadBalancers = loadBalancerProvisioner.provision(app1);
        assertEquals(2, loadBalancers.size());
        List<HostName> activeContainers = tester.getNodes(app1, Node.State.active)
                                                .type(ClusterSpec.Type.container).asList()
                                                .stream()
                                                .map(Node::hostname)
                                                .map(HostName::from)
                                                .sorted()
                                                .collect(Collectors.toList());
        List<HostName> reals = loadBalancers.values().stream()
                                            .flatMap(lb -> lb.reals().stream())
                                            .map(Real::hostname)
                                            .sorted()
                                            .collect(Collectors.toList());
        assertEquals(activeContainers, reals);

        // Removing load balancer with active containers fails
        try {
            loadBalancerProvisioner.deactivate(app1);
            fail("Expected exception");
        } catch (IllegalArgumentException ignored) {}

        // Application and load balancer is removed
        NestedTransaction removeTransaction = new NestedTransaction();
        tester.provisioner().remove(removeTransaction, app1);
        removeTransaction.commit();

        loadBalancerProvisioner.deactivate(app1);
        List<LoadBalancer> assignedLoadBalancer = tester.nodeRepository().database().readLoadBalancers(app1);
        assertEquals(2, loadBalancers.size());
        assertTrue("Load balancers marked for deletion", assignedLoadBalancer.stream().allMatch(LoadBalancer::inactive));
    }

    private ClusterSpec clusterRequest(ClusterSpec.Type type, ClusterSpec.Id id) {
        return ClusterSpec.request(type, id, Version.fromString("6.42"), false);
    }

    private Set<HostSpec> prepare(ApplicationId application, ClusterSpec... specs) {
        tester.makeReadyNodes(specs.length * 2, "default");
        Set<HostSpec> allNodes = new LinkedHashSet<>();
        for (ClusterSpec spec : specs) {
            allNodes.addAll(tester.prepare(application, spec, 2, 1, "default"));
        }
        return allNodes;
    }

}
