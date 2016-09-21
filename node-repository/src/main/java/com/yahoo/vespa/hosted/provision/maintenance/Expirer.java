// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.provision.maintenance;

import com.yahoo.config.provision.NodeType;
import com.yahoo.vespa.hosted.provision.Node;
import com.yahoo.vespa.hosted.provision.NodeRepository;
import com.yahoo.vespa.hosted.provision.node.History;

import java.time.Clock;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.logging.Logger;

/**
 * Superclass of expiry tasks which moves nodes from some state to the dirty state.
 * These jobs runs at least every 25 minutes.
 *
 * @author bratseth
 */
public abstract class Expirer extends Maintainer {

    protected static final Logger log = Logger.getLogger(Expirer.class.getName());

    /** The state to expire from */
    private final Node.State fromState;

    /** The event record type which contains the timestamp to use for expiry */
    private final History.Event.Type eventType;

    private final Clock clock;

    private final Duration expiryTime;

    public Expirer(Node.State fromState, History.Event.Type eventType, NodeRepository nodeRepository, Clock clock, Duration expiryTime) {
        super(nodeRepository, min(Duration.ofMinutes(25), expiryTime));
        this.fromState = fromState;
        this.eventType = eventType;
        this.clock = clock;
        this.expiryTime = expiryTime;
    }

    private static Duration min(Duration a, Duration b) {
        return a.toMillis() < b.toMillis() ? a : b;
    }

    @Override
    protected void maintain() {
        List<Node> expired = new ArrayList<>();
        for (Node node : nodeRepository().getNodes(fromState)) {
            Optional<History.Event> event = node.history().event(eventType);
            if (event.isPresent() && event.get().at().plus(expiryTime).isBefore(clock.instant()))
                expired.add(node);
        }
        if ( ! expired.isEmpty())
            log.info(fromState + " expirer found " + expired.size() + " expired nodes");
        expire(expired);
    }

    @Override
    public String toString() { return "Expiry from " + fromState; }

    /** Implement this callback to take action to expire these nodes */
    protected abstract void expire(List<Node> node);

}
