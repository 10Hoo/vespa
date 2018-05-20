// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.maintenance;

import com.yahoo.component.Version;
import com.yahoo.config.provision.ApplicationId;
import com.yahoo.vespa.hosted.controller.Controller;
import com.yahoo.vespa.hosted.controller.api.integration.configserver.Node;
import com.yahoo.vespa.hosted.controller.api.integration.zone.ZoneId;
import com.yahoo.vespa.hosted.controller.application.SystemApplication;
import com.yahoo.vespa.hosted.controller.versions.VespaVersion;
import com.yahoo.yolean.Exceptions;

import java.time.Duration;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.function.Function;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Maintenance job which upgrades system applications.
 *
 * @author mpolden
 */
public class SystemUpgrader extends Maintainer {

    private static final Logger log = Logger.getLogger(SystemUpgrader.class.getName());

    public SystemUpgrader(Controller controller, Duration interval, JobControl jobControl) {
        super(controller, interval, jobControl);
    }

    @Override
    protected void maintain() {
        Optional<Version> target = targetVersion();
        if (!target.isPresent()) {
            return;
        }
        deploy(SystemApplication.all(), target.get());
    }

    /** Deploy a list of system applications on given version */
    private void deploy(List<SystemApplication> applications, Version target) {
        for (List<ZoneId> zones : controller().zoneRegistry().upgradePolicy().asList()) {
            int done = 0;
            for (SystemApplication application : applications)
                if (application.prerequisites().stream().allMatch(prerequisite -> deploy(zones, prerequisite, target))
                        && deploy(zones, application, target))
                    done++;
            if (done < applications.size()) return;
        }
    }

    /** Deploy application on given version. Returns true when all allocated nodes are on requested version */
    private boolean deploy(List<ZoneId> zones, SystemApplication application, Version target) {
        boolean completed = true;
        for (ZoneId zone : zones) {
            if (!wantedVersion(zone, application.id(), target).equals(target)) {
                log.info(String.format("Deploying %s version %s in %s", application.id(), target, zone));
                controller().applications().deploy(application, zone, target);
            }
            completed = completed && currentVersion(zone, application.id(), target).equals(target);
        }
        return completed;
    }

    private Version wantedVersion(ZoneId zone, ApplicationId application, Version defaultVersion) {
        return minVersion(zone, application, Node::wantedVersion).orElse(defaultVersion);
    }

    private Version currentVersion(ZoneId zone, ApplicationId application, Version defaultVersion) {
        return minVersion(zone, application, Node::currentVersion).orElse(defaultVersion);
    }

    private Optional<Version> minVersion(ZoneId zone, ApplicationId application, Function<Node, Version> versionField) {
        try {
            return controller().configServer()
                               .nodeRepository()
                               .listOperational(zone, application)
                               .stream()
                               .map(versionField)
                               .min(Comparator.naturalOrder());
        } catch (Exception e) {
            log.log(Level.WARNING, String.format("Failed to get version for %s in %s: %s", application, zone,
                                                 Exceptions.toMessageString(e)));
            return Optional.empty();
        }
    }

    /** Returns target version for the system */
    private Optional<Version> targetVersion() {
        return controller().versionStatus().controllerVersion()
                           .filter(vespaVersion -> !vespaVersion.isSystemVersion())
                           .map(VespaVersion::versionNumber);
    }

}
