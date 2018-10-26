// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.integrationTests;

import com.yahoo.config.provision.NodeType;
import com.yahoo.vespa.hosted.dockerapi.ContainerName;
import com.yahoo.vespa.hosted.dockerapi.ContainerResources;
import com.yahoo.vespa.hosted.dockerapi.DockerImage;
import com.yahoo.vespa.hosted.node.admin.configserver.noderepository.NodeSpec;
import com.yahoo.vespa.hosted.node.admin.nodeadmin.NodeAdminStateUpdaterImpl;
import com.yahoo.vespa.hosted.provision.Node;
import org.junit.Test;

import java.util.Arrays;
import java.util.OptionalLong;

import static com.yahoo.vespa.hosted.node.admin.integrationTests.DockerTester.HOST_HOSTNAME;
import static com.yahoo.vespa.hosted.node.admin.integrationTests.DockerTester.NODE_PROGRAM;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;

/**
 * Tests rebooting of Docker host
 *
 * @author musum
 */
public class RebootTest {

    private final String hostname = "host1.test.yahoo.com";
    private final DockerImage dockerImage = new DockerImage("dockerImage");

    @Test
    public void test() {
        try (DockerTester tester = new DockerTester()) {
            tester.addChildNodeRepositoryNode(createNodeRepositoryNode());

            tester.inOrder(tester.docker).createContainerCommand(
                    eq(dockerImage), eq(ContainerResources.from(0, 0)), eq(new ContainerName("host1")), eq(hostname));

            try {
                tester.setWantedState(NodeAdminStateUpdaterImpl.State.SUSPENDED);
            } catch (RuntimeException ignored) { }

            tester.inOrder(tester.orchestrator).suspend(
                    eq(HOST_HOSTNAME.value()), eq(Arrays.asList(hostname, HOST_HOSTNAME.value())));
            tester.inOrder(tester.docker).executeInContainerAsUser(
                    eq(new ContainerName("host1")), eq("root"), eq(OptionalLong.empty()), eq(NODE_PROGRAM), eq("stop"));
            assertTrue(tester.nodeAdmin.setFrozen(true));
        }
    }

    private NodeSpec createNodeRepositoryNode() {
        return new NodeSpec.Builder()
                .hostname(hostname)
                .wantedDockerImage(dockerImage)
                .state(Node.State.active)
                .nodeType(NodeType.tenant)
                .flavor("docker")
                .vespaVersion("6.50.0")
                .wantedRestartGeneration(1L)
                .currentRestartGeneration(1L)
                .build();
    }
}
