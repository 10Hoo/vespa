// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.provision.maintenance;

import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.Deployer;
import com.yahoo.config.provision.Deployment;
import com.yahoo.transaction.Mutex;
import com.yahoo.vespa.hosted.provision.Node;
import com.yahoo.vespa.hosted.provision.NodeRepository;

import java.time.Duration;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.function.Function;
import java.util.logging.Level;
import java.util.stream.Collectors;

/**
 * The application maintainer regularly redeploys all applications.
 * This is necessary because applications may gain and lose active nodes due to nodes being moved to and from the
 * failed state. This is corrected by redeploying the applications periodically.
 * It can not (at this point) be done reliably synchronously as part of the fail/reactivate call due to the need for this
 * to happen at a node having the deployer.
 *
 * @author bratseth
 */
public class ApplicationMaintainer extends Maintainer {

    private final Deployer deployer;

    private final Executor deploymentExecutor = Executors.newCachedThreadPool();
    
    public ApplicationMaintainer(Deployer deployer, NodeRepository nodeRepository, Duration interval) {
        super(nodeRepository, interval);
        this.deployer = deployer;
    }

    @Override
    protected void maintain() {
        Set<ApplicationId> applications = activeApplications();
        for (ApplicationId application : applications) {
            try {
                // An application might change it's state between the time the set of applications is retrieved and the
                // time deployment happens. Lock on application and check if it's still active.
                //
                // Lock is acquired with a low timeout to reduce the chance of colliding with an external deployment.
                try (Mutex lock = nodeRepository().lock(application, Duration.ofSeconds(1))) {
                    if ( ! isActive(application)) continue; // became inactive since we started the loop
                    Optional<Deployment> deployment = deployer.deployFromLocalActive(application, Duration.ofMinutes(30));
                    if ( ! deployment.isPresent()) continue; // this will be done at another config server

                    // deploy asynchronously to make sure we do all applications even when deployments are slow
                    deployAsynchronously(deployment.get()); 
                }
                throttle(applications.size());
            }
            catch (RuntimeException e) {
                log.log(Level.WARNING, "Exception on maintenance redeploy of " + application, e);
            }
        }
    }
    
    protected void deployAsynchronously(Deployment deployment) {
        deploymentExecutor.execute(() -> {
            try {
                deployment.activate();
            }
            catch (RuntimeException e) {
                log.log(Level.WARNING, "Exception on maintenance redeploy", e);
            }
        });
    }
    
    protected void throttle(int applicationCount) {
        // Sleep for a length of time that will spread deployment evenly over the maintenance period
        try { Thread.sleep(interval().toMillis() / applicationCount); } catch (InterruptedException e) { return; }
    }

    protected Set<ApplicationId> activeApplications() {
        return nodeRepository().getNodes(Node.State.active).stream()
                .map(node -> node.allocation().get().owner())
                .collect(Collectors.toCollection(LinkedHashSet::new));
    }

    private boolean isActive(ApplicationId application) {
        return ! nodeRepository().getNodes(application, Node.State.active).isEmpty();
    }

    @Override
    public String toString() { return "Periodic application redeployer"; }

}
