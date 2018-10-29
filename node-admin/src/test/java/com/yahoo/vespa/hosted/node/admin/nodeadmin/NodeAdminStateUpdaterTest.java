// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.nodeadmin;

import com.yahoo.config.provision.HostName;
import com.yahoo.config.provision.NodeType;
import com.yahoo.vespa.hosted.node.admin.configserver.noderepository.NodeSpec;
import com.yahoo.vespa.hosted.node.admin.configserver.noderepository.NodeRepository;
import com.yahoo.vespa.hosted.node.admin.configserver.orchestrator.Orchestrator;
import com.yahoo.vespa.hosted.provision.Node;
import org.junit.Test;

import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

import static com.yahoo.vespa.hosted.node.admin.nodeadmin.NodeAdminStateUpdater.State.RESUMED;
import static com.yahoo.vespa.hosted.node.admin.nodeadmin.NodeAdminStateUpdater.State.SUSPENDED;
import static com.yahoo.vespa.hosted.node.admin.nodeadmin.NodeAdminStateUpdater.State.SUSPENDED_NODE_ADMIN;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * Basic test of NodeAdminStateUpdater
 *
 * @author freva
 */
public class NodeAdminStateUpdaterTest {
    private final NodeRepository nodeRepository = mock(NodeRepository.class);
    private final Orchestrator orchestrator = mock(Orchestrator.class);
    private final NodeAdmin nodeAdmin = mock(NodeAdmin.class);
    private final HostName hostHostname = HostName.from("basehost1.test.yahoo.com");

    private final NodeAdminStateUpdater refresher = spy(new NodeAdminStateUpdater(
            nodeRepository, orchestrator, nodeAdmin, hostHostname));


    @Test
    public void state_convergence() {
        mockNodeRepo(Node.State.active, 4);
        List<String> activeHostnames = nodeRepository.getNodes(hostHostname.value()).stream()
                .map(NodeSpec::getHostname)
                .collect(Collectors.toList());
        List<String> suspendHostnames = new ArrayList<>(activeHostnames);
        suspendHostnames.add(hostHostname.value());
        when(nodeAdmin.subsystemFreezeDuration()).thenReturn(Duration.ofSeconds(1));

        {
            // Initially everything is frozen to force convergence
            assertResumeStateError(RESUMED, "NodeAdmin is not yet unfrozen");
            when(nodeAdmin.setFrozen(eq(false))).thenReturn(true);
            refresher.converge(RESUMED);
            verify(orchestrator, times(1)).resume(hostHostname.value());

            // We are already resumed, so this should return without resuming again
            refresher.converge(RESUMED);
            verify(orchestrator, times(1)).resume(hostHostname.value());
            verify(nodeAdmin, times(2)).setFrozen(eq(false));

            // Lets try to suspend node admin only
            when(nodeAdmin.setFrozen(eq(true))).thenReturn(false);
            assertResumeStateError(SUSPENDED_NODE_ADMIN, "NodeAdmin is not yet frozen");
            verify(nodeAdmin, times(2)).setFrozen(eq(false));
        }

        {
            // First orchestration failure happens within the freeze convergence timeout,
            // and so should not call setFrozen(false)
            final String exceptionMessage = "Cannot allow to suspend because some reason";
            when(nodeAdmin.setFrozen(eq(true))).thenReturn(true);
            doThrow(new RuntimeException(exceptionMessage)).doNothing()
                    .when(orchestrator).suspend(eq(hostHostname.value()));
            assertResumeStateError(SUSPENDED_NODE_ADMIN, exceptionMessage);
            verify(nodeAdmin, times(2)).setFrozen(eq(false));

            refresher.converge(SUSPENDED_NODE_ADMIN);
            verify(nodeAdmin, times(2)).setFrozen(eq(false));
        }

        {
            // At this point orchestrator will say its OK to suspend, but something goes wrong when we try to stop services
            final String exceptionMessage = "Failed to stop services";
            verify(orchestrator, times(0)).suspend(eq(hostHostname.value()), eq(suspendHostnames));
            doThrow(new RuntimeException(exceptionMessage)).doNothing().when(nodeAdmin).stopNodeAgentServices(eq(activeHostnames));
            assertResumeStateError(SUSPENDED, exceptionMessage);
            verify(orchestrator, times(1)).suspend(eq(hostHostname.value()), eq(suspendHostnames));
            // Make sure we dont roll back if we fail to stop services - we will try to stop again next tick
            verify(nodeAdmin, times(2)).setFrozen(eq(false));

            // Finally we are successful in transitioning to frozen
            refresher.converge(SUSPENDED);
        }
    }

    @Test
    public void half_transition_revert() {
        final String exceptionMsg = "Cannot allow to suspend because some reason";
        mockNodeRepo(Node.State.active, 3);

        // Initially everything is frozen to force convergence
        when(nodeAdmin.setFrozen(eq(false))).thenReturn(true);
        refresher.converge(RESUMED);
        verify(nodeAdmin, times(1)).setFrozen(eq(false));

        // Let's start suspending, we are able to freeze the nodes, but orchestrator denies suspension
        when(nodeAdmin.subsystemFreezeDuration()).thenReturn(Duration.ofSeconds(1));
        when(nodeAdmin.setFrozen(eq(true))).thenReturn(true);
        doThrow(new RuntimeException(exceptionMsg)).when(orchestrator).suspend(eq(hostHostname.value()));

        assertResumeStateError(SUSPENDED_NODE_ADMIN, exceptionMsg);
        verify(nodeAdmin, times(1)).setFrozen(eq(true));
        assertResumeStateError(SUSPENDED_NODE_ADMIN, exceptionMsg);
        verify(nodeAdmin, times(2)).setFrozen(eq(true));
        assertResumeStateError(SUSPENDED_NODE_ADMIN, exceptionMsg);
        verify(nodeAdmin, times(3)).setFrozen(eq(true));
        verify(nodeAdmin, times(1)).setFrozen(eq(false)); // No new unfreezes during last 2 ticks
        verify(nodeAdmin, times(1)).refreshContainersToRun(any());

        // Only resume and fetch containers when subsystem freeze duration expires
        when(nodeAdmin.subsystemFreezeDuration()).thenReturn(Duration.ofHours(1));
        assertResumeStateError(SUSPENDED_NODE_ADMIN, exceptionMsg);
        verify(nodeAdmin, times(2)).setFrozen(eq(false));
        verify(nodeAdmin, times(2)).refreshContainersToRun(any());

        // We change our mind, want to remain resumed
        refresher.converge(RESUMED);
        verify(nodeAdmin, times(3)).setFrozen(eq(false)); // Make sure that we unfreeze!
    }

    @Test
    public void do_not_orchestrate_host_when_not_active() {
        when(nodeAdmin.subsystemFreezeDuration()).thenReturn(Duration.ofHours(1));
        when(nodeAdmin.setFrozen(anyBoolean())).thenReturn(true);
        mockNodeRepo(Node.State.ready, 3);

        // Resume and suspend only require that node-agents are frozen and permission from
        // orchestrator to resume/suspend host. Therefore, if host is not active, we only need to freeze.
        refresher.converge(RESUMED);
        verify(orchestrator, never()).resume(eq(hostHostname.value()));

        refresher.converge(SUSPENDED_NODE_ADMIN);
        verify(orchestrator, never()).suspend(eq(hostHostname.value()));

        // When doing batch suspend, only suspend the containers if the host is not active
        List<String> activeHostnames = nodeRepository.getNodes(hostHostname.value()).stream()
                .map(NodeSpec::getHostname)
                .collect(Collectors.toList());
        refresher.converge(SUSPENDED);
        verify(orchestrator, times(1)).suspend(eq(hostHostname.value()), eq(activeHostnames));
    }

    private void assertResumeStateError(NodeAdminStateUpdater.State targetState, String reason) {
        try {
            refresher.converge(targetState);
            fail("Expected set resume state to fail with \"" + reason + "\", but it succeeded without error");
        } catch (RuntimeException e) {
            assertEquals(reason, e.getMessage());
        }
    }

    private void mockNodeRepo(Node.State hostState, int numberOfNodes) {
        List<NodeSpec> containersToRun = IntStream.range(0, numberOfNodes)
                .mapToObj(i -> new NodeSpec.Builder()
                        .hostname("host" + i + ".test.yahoo.com")
                        .state(Node.State.active)
                        .nodeType(NodeType.tenant)
                        .flavor("docker")
                        .minCpuCores(1)
                        .minMainMemoryAvailableGb(1)
                        .minDiskAvailableGb(1)
                        .build())
                .collect(Collectors.toList());

        when(nodeRepository.getNodes(eq(hostHostname.value()))).thenReturn(containersToRun);

        when(nodeRepository.getNode(eq(hostHostname.value()))).thenReturn(new NodeSpec.Builder()
                .hostname(hostHostname.value())
                .state(hostState)
                .nodeType(NodeType.tenant)
                .flavor("default")
                .minCpuCores(1)
                .minMainMemoryAvailableGb(1)
                .minDiskAvailableGb(1)
                .build());
    }
}
