// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.orchestrator.restapi.wire;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import javax.annotation.concurrent.Immutable;
import java.util.Collections;
import java.util.List;

@Immutable
public class BatchHostSuspendRequest {
    public static final String PARENT_HOSTNAME_FIELD = "parentHostname";
    public static final String HOSTNAMES_FIELD = "hostnames";

    public final String parentHostname;
    public final List<String> hostnames;

    @JsonCreator
    public BatchHostSuspendRequest(
            @JsonProperty(PARENT_HOSTNAME_FIELD) String parentHostname,
            @JsonProperty(HOSTNAMES_FIELD) List<String> hostnames) {
        this.parentHostname = parentHostname;
        this.hostnames = Collections.unmodifiableList(hostnames);
    }

    /**
     * @return The hostname of the parent of the hostnames, if applicable, which can be used for debugging.
     */
    @JsonProperty(PARENT_HOSTNAME_FIELD)
    public String getParentHostname() {
        return parentHostname;
    }

    @JsonProperty(HOSTNAMES_FIELD)
    public List<String> getHostnames() {
        return hostnames;
    }

    @Override
    public String toString() {
        return "BatchHostSuspendRequest{" +
                "parentHostname=" + parentHostname +
                ", hostnames=" + hostnames +
                '}';
    }
}
