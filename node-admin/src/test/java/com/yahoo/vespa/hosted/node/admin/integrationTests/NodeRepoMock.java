// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.integrationTests;

import com.yahoo.config.provision.NodeType;
import com.yahoo.vespa.hosted.node.admin.NodeAcl;
import com.yahoo.vespa.hosted.node.admin.NodeRepositoryNode;
import com.yahoo.vespa.hosted.node.admin.nodeagent.NodeAttributes;
import com.yahoo.vespa.hosted.node.admin.configserver.noderepository.NodeRepository;
import com.yahoo.vespa.hosted.provision.Node;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * Mock with some simple logic
 *
 * @author dybis
 */
public class NodeRepoMock implements NodeRepository {
    private static final Object monitor = new Object();

    private final Map<String, NodeRepositoryNode> nodeRepositoryNodesByHostname = new HashMap<>();
    private final Map<String, List<NodeAcl>> acls = new HashMap<>();
    private final CallOrderVerifier callOrderVerifier;

    public NodeRepoMock(CallOrderVerifier callOrderVerifier) {
        this.callOrderVerifier = callOrderVerifier;
    }

    @Override
    public List<NodeRepositoryNode> getNodes(String baseHostName) {
        synchronized (monitor) {
            return new ArrayList<>(nodeRepositoryNodesByHostname.values());
        }
    }

    @Override
    public List<NodeRepositoryNode> getNodes(NodeType... nodeTypes) {
        return Collections.emptyList();
    }

    @Override
    public Optional<NodeRepositoryNode> getNode(String hostName) {
        synchronized (monitor) {
            return Optional.ofNullable(nodeRepositoryNodesByHostname.get(hostName));
        }
    }

    @Override
    public List<NodeAcl> getNodeAcl(String hostName) {
        synchronized (monitor) {
            return Optional.ofNullable(acls.get(hostName))
                    .orElseGet(Collections::emptyList);
        }
    }

    @Override
    public void updateNodeAttributes(String hostName, NodeAttributes nodeAttributes) {
        synchronized (monitor) {
            callOrderVerifier.add("updateNodeAttributes with HostName: " + hostName + ", " + nodeAttributes);
        }
    }

    @Override
    public void setNodeState(String hostName, Node.State nodeState) {
        Optional<NodeRepositoryNode> node = getNode(hostName);

        synchronized (monitor) {
            node.ifPresent(nrn -> updateNodeRepositoryNode(new NodeRepositoryNode.Builder(nrn)
                    .nodeState(nodeState)
                    .build()));
            callOrderVerifier.add("setNodeState " + hostName + " to " + nodeState);
        }
    }

    public void updateNodeRepositoryNode(NodeRepositoryNode nodeRepositoryNode) {
        nodeRepositoryNodesByHostname.put(nodeRepositoryNode.hostname, nodeRepositoryNode);
    }

    public int getNumberOfContainerSpecs() {
        synchronized (monitor) {
            return nodeRepositoryNodesByHostname.size();
        }
    }
}
