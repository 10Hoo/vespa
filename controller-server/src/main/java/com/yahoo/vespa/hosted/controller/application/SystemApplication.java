// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.application;

import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.NodeType;

import java.util.Arrays;
import java.util.List;

/**
 * This represents a system-level application in hosted Vespa. E.g. the zone-application.
 *
 * @author mpolden
 */
public enum  SystemApplication {

    configServer(ApplicationId.from("hosted-vespa", "zone-config-servers", "default"), NodeType.config),
    zone(ApplicationId.from("hosted-vespa", "routing", "default"), NodeType.proxy);

    private final ApplicationId id;
    private final NodeType nodeType;

    SystemApplication(ApplicationId id, NodeType nodeType) {
        this.id = id;
        this.nodeType = nodeType;
    }

    public ApplicationId id() {
        return id;
    }

    /** The type of nodes that will be allocated to this */
    public NodeType nodeType() {
        return nodeType;
    }

    /** Returns whether this system application has its own application package */
    public boolean hasApplicationPackage() {
        return nodeType == NodeType.proxy;
    }

    /** All known system applications */
    public static List<SystemApplication> all() {
        return Arrays.asList(values());
    }

    @Override
    public String toString() {
        return String.format("system application %s of type %s", id, nodeType);
    }

}
