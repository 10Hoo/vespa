// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.provision.restapi.v2;

import com.yahoo.container.jdisc.HttpRequest;
import com.yahoo.container.jdisc.HttpResponse;
import com.yahoo.slime.Cursor;
import com.yahoo.slime.Slime;
import com.yahoo.vespa.config.SlimeUtils;
import com.yahoo.vespa.hosted.provision.Node;
import com.yahoo.vespa.hosted.provision.NodeRepository;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.List;

/**
 * @author mpolden
 */
public class NodeAclResponse extends HttpResponse {

    private final NodeRepository nodeRepository;
    private final Slime slime;

    public NodeAclResponse(HttpRequest request, NodeRepository nodeRepository) {
        super(200);
        this.nodeRepository = nodeRepository;
        this.slime = new Slime();

        final Cursor root = slime.setObject();
        final String hostname = baseName(request.getUri().getPath());
        toSlime(hostname, root);
    }

    private static String baseName(String path) {
        return new File(path).getName();
    }

    private void toSlime(String hostname, Cursor object) {
        Node node = nodeRepository.getNode(hostname)
                .orElseThrow(() -> new IllegalArgumentException("No node with hostname '" + hostname + "'"));

        toSlime(nodeRepository.getTrustedNodes(node), object.setArray("trustedNodes"));
    }

    private void toSlime(List<Node> trustedNodes, Cursor array) {
        trustedNodes.forEach(node -> {
            Cursor object = array.addObject();
            object.setString("hostname", node.hostname());
            object.setString("ipAddress", node.ipAddress().get());
        });
    }

    @Override
    public void render(OutputStream outputStream) throws IOException {
        outputStream.write(SlimeUtils.toJsonBytes(slime));
    }

    @Override
    public String getContentType() {
        return "application/json";
    }
}
