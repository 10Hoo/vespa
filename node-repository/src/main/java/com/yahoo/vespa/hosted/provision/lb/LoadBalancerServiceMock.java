// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.provision.lb;

import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.ClusterSpec;
import com.yahoo.config.provision.HostName;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author mpolden
 */
public class LoadBalancerServiceMock implements LoadBalancerService {

    private final Map<LoadBalancerId, LoadBalancer> loadBalancers = new HashMap<>();

    @Override
    public Protocol protocol() {
        return Protocol.ipv4;
    }

    @Override
    public LoadBalancer create(ApplicationId application, ClusterSpec.Id cluster, List<Real> reals) {
        LoadBalancer loadBalancer = new LoadBalancer(
                new LoadBalancerId(application, cluster),
                HostName.from("lb-" + application.toShortString() + "-" + cluster.value()),
                Collections.singletonList(4443),
                reals,
                false);
        loadBalancers.put(loadBalancer.id(), loadBalancer);
        return loadBalancer;
    }

    @Override
    public void remove(LoadBalancerId loadBalancer) {
        loadBalancers.remove(loadBalancer);
    }

}
