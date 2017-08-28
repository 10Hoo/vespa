// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.server.provision;

import com.yahoo.config.model.api.HostProvisioner;
import com.yahoo.config.provision.*;

import java.util.List;
import java.util.stream.Collectors;

/**
 * Host provisioning from an existing {@link AllocatedHosts} instance.
 *
 * @author bratseth
 */
public class StaticProvisioner implements HostProvisioner {

    private final AllocatedHosts allocatedHosts;

    public StaticProvisioner(AllocatedHosts allocatedHosts) {
        this.allocatedHosts = allocatedHosts;
    }

    @Override
    public HostSpec allocateHost(String alias) {
        throw new UnsupportedOperationException("Allocating a single host from provisioning info is not supported");
    }

    @Override
    public List<HostSpec> prepare(ClusterSpec cluster, Capacity capacity, int groups, ProvisionLogger logger) {
        return allocatedHosts.getHosts().stream()
                .filter(host -> host.membership().isPresent() && matches(host.membership().get().cluster(), cluster))
                .collect(Collectors.toList());
    }

    private boolean matches(ClusterSpec nodeCluster, ClusterSpec requestedCluster) {
        if (requestedCluster.group().isPresent()) // we are requesting a specific group
            return nodeCluster.equals(requestedCluster);
        else // we are requesting nodes of all groups in this cluster
            return nodeCluster.equalsIgnoringGroupAndVespaVersion(requestedCluster);
    }

}
