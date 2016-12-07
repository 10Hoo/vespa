// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.integrationTests;

import com.yahoo.vespa.hosted.dockerapi.ContainerName;
import com.yahoo.vespa.hosted.dockerapi.DockerImage;
import com.yahoo.vespa.hosted.node.admin.ContainerNodeSpec;
import com.yahoo.vespa.hosted.node.admin.docker.DockerOperationsImpl;
import com.yahoo.vespa.hosted.provision.Node;
import org.junit.Test;

import java.net.UnknownHostException;

/**
 * Tests that different wanted and current restart generation leads to execution of restart command
 *
 * @author musum
 */
public class RestartTest {

    @Test
    public void test() throws InterruptedException, UnknownHostException {
        try (DockerTester dockerTester = new DockerTester()) {

            long wantedRestartGeneration = 1;
            long currentRestartGeneration = wantedRestartGeneration;
            dockerTester.addContainerNodeSpec(createContainerNodeSpec(wantedRestartGeneration, currentRestartGeneration));

            // Wait for node admin to be notified with node repo state and the docker container has been started
            while (dockerTester.getNodeAdmin().getListOfHosts().size() == 0) {
                Thread.sleep(10);
            }

            CallOrderVerifier callOrderVerifier = dockerTester.getCallOrderVerifier();
            // Check that the container is started and NodeRepo has received the PATCH update
            callOrderVerifier.assertInOrder("createContainerCommand with DockerImage: DockerImage { imageId=dockerImage }, HostName: host1, ContainerName: ContainerName { name=container }",
                                            "updateNodeAttributes with HostName: host1, NodeAttributes: NodeAttributes{restartGeneration=1, rebootGeneration=0, dockerImage=DockerImage { imageId=dockerImage }, vespaVersion=''}");

            wantedRestartGeneration = 2;
            currentRestartGeneration = 1;
            dockerTester.updateContainerNodeSpec(createContainerNodeSpec(wantedRestartGeneration, currentRestartGeneration));

            callOrderVerifier.assertInOrder("Suspend for host1",
                                            "executeInContainer with ContainerName: ContainerName { name=container }, args: [" + DockerOperationsImpl.NODE_PROGRAM + ", restart]");
        }
    }

    private ContainerNodeSpec createContainerNodeSpec(long wantedRestartGeneration, long currentRestartGeneration) {
        return new ContainerNodeSpec.Builder()
                .hostname("host1")
                .wantedDockerImage(new DockerImage("dockerImage"))
                .containerName(new ContainerName("container"))
                .nodeState(Node.State.active)
                .nodeType("tenant")
                .nodeFlavor("docker")
                .wantedRestartGeneration(wantedRestartGeneration)
                .currentRestartGeneration(currentRestartGeneration)
                .build();
    }
}
