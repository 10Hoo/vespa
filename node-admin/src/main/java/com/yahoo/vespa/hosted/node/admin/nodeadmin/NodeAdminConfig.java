// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.nodeadmin;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.File;
import java.io.IOException;
import java.util.logging.Logger;

@JsonIgnoreProperties(ignoreUnknown = true)
public class NodeAdminConfig {
    private static final Logger logger = Logger.getLogger(NodeAdminConfig.class.getName());
    private static final ObjectMapper mapper = new ObjectMapper();

    public enum Mode {
        aws_tenant,
        config_server_host,
        tenant,
    }

    @JsonProperty("mode")
    public Mode mode = Mode.tenant;

    @JsonProperty("docker")
    public DockerAdminConfig docker = new DockerAdminConfig();

    public static NodeAdminConfig fromFile(File file) {
        if (!file.exists()) {
            return new NodeAdminConfig();
        }

        try {
            return mapper.readValue(file, NodeAdminConfig.class);
        } catch (IOException e) {
            throw new RuntimeException("Failed to read " + file + " as a " +
                    NodeAdminConfig.class.getName(), e);
        }
    }
}
