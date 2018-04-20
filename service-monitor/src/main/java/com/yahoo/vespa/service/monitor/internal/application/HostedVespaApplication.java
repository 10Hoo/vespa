// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.service.monitor.internal.application;

import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.Capacity;
import com.yahoo.config.provision.ClusterSpec;
import com.yahoo.config.provision.NodeType;
import com.yahoo.config.provision.TenantName;
import com.yahoo.component.Version;

/**
 * @author freva
 */
public abstract class HostedVespaApplication {

    public static final TenantName TENANT_NAME = TenantName.from("hosted-vespa");

    protected final ApplicationId applicationId;
    protected final Capacity capacity;
    protected final ClusterSpec.Type clusterType;
    protected final ClusterSpec.Id clusterId;
    protected final ClusterSpec.Group clusterGroup;

    protected HostedVespaApplication(String applicationName, NodeType nodeType,
                                     ClusterSpec.Type clusterType, ClusterSpec.Id clusterId, ClusterSpec.Group clusterGroup) {
        this(createHostedVespaApplicationId(applicationName), Capacity.fromRequiredNodeType(nodeType), clusterType, clusterId, clusterGroup);
    }

    protected HostedVespaApplication(ApplicationId applicationId, Capacity capacity,
                                     ClusterSpec.Type clusterType, ClusterSpec.Id clusterId, ClusterSpec.Group clusterGroup) {
        this.applicationId = applicationId;
        this.capacity = capacity;
        this.clusterType = clusterType;
        this.clusterId = clusterId;
        this.clusterGroup = clusterGroup;
    }

    public ApplicationId getApplicationId() {
        return applicationId;
    }

    public Capacity getCapacity() {
        return capacity;
    }

    public ClusterSpec getClusterSpecWithVersion(Version version) {
        return ClusterSpec.from(clusterType, clusterId, clusterGroup, version, true);
    }


    public static ApplicationId createHostedVespaApplicationId(String applicationName) {
        return new ApplicationId.Builder()
                .tenant(TENANT_NAME)
                .applicationName(applicationName)
                .build();
    }
}
