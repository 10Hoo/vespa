// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.orchestrator.controller;

import com.yahoo.vespa.applicationmodel.ApplicationInstance;
import com.yahoo.vespa.applicationmodel.ClusterId;
import com.yahoo.vespa.applicationmodel.HostName;
import com.yahoo.vespa.applicationmodel.ServiceInstance;
import com.yahoo.vespa.orchestrator.DummyInstanceLookupService;
import com.yahoo.vespa.orchestrator.VespaModelUtil;
import com.yahoo.vespa.service.monitor.ServiceMonitorStatus;

import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * Mock implementation of ClusterControllerClient
 * <p>
 *
 * @author smorgrav
 */
public class ClusterControllerClientFactoryMock implements ClusterControllerClientFactory {
    Map<String, ClusterControllerState> nodes = new HashMap<>();

    public boolean isInMaintenance(ApplicationInstance<ServiceMonitorStatus> appInstance, HostName hostName) {
        try {
            ClusterId clusterName = VespaModelUtil.getContentClusterName(appInstance, hostName);
            int storageNodeIndex = VespaModelUtil.getStorageNodeIndex(appInstance, hostName);
            String globalMapKey = clusterName.s() + storageNodeIndex;
            return nodes.getOrDefault(globalMapKey, ClusterControllerState.UP) == ClusterControllerState.MAINTENANCE;
        } catch (Exception e) {
            //Catch all - meant to catch cases where the node is not part of a storage cluster
            return false;
        }
    }

    public void setAllDummyNodesAsUp() {
        for (ApplicationInstance<ServiceMonitorStatus> app : DummyInstanceLookupService.getApplications()) {
            Set<HostName> hosts = DummyInstanceLookupService.getContentHosts(app.reference());
            for (HostName host : hosts) {
                ClusterId clusterName = VespaModelUtil.getContentClusterName(app, host);
                int storageNodeIndex = VespaModelUtil.getStorageNodeIndex(app, host);
                String globalMapKey = clusterName.s() + storageNodeIndex;
                nodes.put(globalMapKey, ClusterControllerState.UP);
            }
        }
    }

    @Override
    public ClusterControllerClient createClient(Collection<? extends ServiceInstance<?>> clusterControllers, String clusterName) {
        return new ClusterControllerClient() {

            @Override
            public ClusterControllerStateResponse setNodeState(int storageNodeIndex, ClusterControllerState wantedState) throws IOException {
                nodes.put(clusterName + storageNodeIndex, wantedState);
                return new ClusterControllerStateResponse(true, "Yes");
            }

            @Override
            public ClusterControllerStateResponse setApplicationState(ClusterControllerState wantedState) throws IOException {
                Set<String> keyCopy = new HashSet<>(nodes.keySet());
                for (String s : keyCopy) {
                    if (s.startsWith(clusterName)) {
                        nodes.put(s, wantedState);
                    }
                }
                return new ClusterControllerStateResponse(true, "It works");
            }
        };
    }
}
