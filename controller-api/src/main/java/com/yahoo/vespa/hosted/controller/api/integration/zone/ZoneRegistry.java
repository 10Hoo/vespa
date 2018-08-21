// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.api.integration.zone;

import com.yahoo.config.provision.Environment;
import com.yahoo.config.provision.RegionName;
import com.yahoo.config.provision.SystemName;
import com.yahoo.vespa.athenz.api.AthenzService;
import com.yahoo.vespa.hosted.controller.api.identifiers.DeploymentId;

import java.net.URI;
import java.time.Duration;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * Provides information about zones in a hosted Vespa system.
 *
 * @author mpolden
 */
public interface ZoneRegistry {

    /** Returns whether the system of this registry contains the given zone */
    boolean hasZone(ZoneId zoneId);

    /** Returns a list containing the id of all zones in this registry */
    ZoneFilter zones();

    /** Returns the default region for the given environment, if one is configured */
    Optional<RegionName> getDefaultRegion(Environment environment);

    /** Returns the API endpoints of all known config servers in the given zone */
    List<URI> getConfigServerUris(ZoneId zoneId);

    /** Returns the URI for the config server VIP in the given zone, or Optional.empty() if no VIP exists */
    default Optional<URI> getConfigServerVipUri(ZoneId zoneId) { return Optional.empty(); }

    /** Returns a URL with the logs for the given deployment, if logging is configured for its zone */
    Optional<URI> getLogServerUri(DeploymentId deploymentId);

    /** Returns the time to live for deployments in the given zone, or empty if this is infinite */
    Optional<Duration> getDeploymentTimeToLive(ZoneId zoneId);

    /** Returns the monitoring system URL for the given deployment */
    URI getMonitoringSystemUri(DeploymentId deploymentId);

    /** Returns the system of this registry */
    SystemName system();

    /** Return the configserver's Athenz service identity */
    AthenzService getConfigServerAthenzService(ZoneId zoneId);

    /** Returns the Vespa upgrade policy to use for zones in this registry */
    UpgradePolicy upgradePolicy();

    /** Returns the OS upgrade policy to use for zones in this registry */
    // TODO: Remove
    default UpgradePolicy osUpgradePolicy() {
        return upgradePolicy();
    }

    // TODO: Remove default implementation
    /** Returns all OS upgrade policies */
    default List<UpgradePolicy> osUpgradePolicies() {
        return Collections.singletonList(upgradePolicy());
    }

    /** Returns the OS upgrade policy to use for zones belonging to given cloud, in this registry */
    default UpgradePolicy osUpgradePolicy(CloudName cloud) {
        return osUpgradePolicy(); // TODO: Remove default implementation
    }

}
