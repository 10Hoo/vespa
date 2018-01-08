// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.deployment;

import com.yahoo.component.Version;
import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.SystemName;
import com.yahoo.vespa.hosted.controller.Application;
import com.yahoo.vespa.hosted.controller.ApplicationController;
import com.yahoo.vespa.hosted.controller.Controller;
import com.yahoo.vespa.hosted.controller.LockedApplication;
import com.yahoo.vespa.hosted.controller.api.integration.zone.ZoneId;
import com.yahoo.vespa.hosted.controller.application.ApplicationList;
import com.yahoo.vespa.hosted.controller.application.Change;
import com.yahoo.vespa.hosted.controller.application.Change.VersionChange;
import com.yahoo.vespa.hosted.controller.application.Deployment;
import com.yahoo.vespa.hosted.controller.application.DeploymentJobs.JobError;
import com.yahoo.vespa.hosted.controller.application.DeploymentJobs.JobReport;
import com.yahoo.vespa.hosted.controller.application.DeploymentJobs.JobType;
import com.yahoo.vespa.hosted.controller.application.JobList;
import com.yahoo.vespa.hosted.controller.application.JobStatus;
import com.yahoo.vespa.hosted.controller.persistence.CuratorDb;

import java.time.Clock;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.logging.Logger;

/**
 * Responsible for scheduling deployment jobs in a build system and keeping
 * Application.deploying() in sync with what is scheduled.
 *
 * This class is multithread safe.
 *
 * @author bratseth
 * @author mpolden
 */
public class DeploymentTrigger {

    /** The max duration a job may run before we consider it dead/hanging */
    private final Duration jobTimeout;

    private final static Logger log = Logger.getLogger(DeploymentTrigger.class.getName());

    private final Controller controller;
    private final Clock clock;
    private final BuildSystem buildSystem;
    private final DeploymentOrder order;

    public DeploymentTrigger(Controller controller, CuratorDb curator, Clock clock) {
        Objects.requireNonNull(controller,"controller cannot be null");
        Objects.requireNonNull(curator,"curator cannot be null");
        Objects.requireNonNull(clock,"clock cannot be null");
        this.controller = controller;
        this.clock = clock;
        this.buildSystem = new PolledBuildSystem(controller, curator);
        this.order = new DeploymentOrder(controller);
        this.jobTimeout = controller.system().equals(SystemName.main) ? Duration.ofHours(12) : Duration.ofHours(1);
    }

    /** Returns the time in the past before which jobs are at this moment considered unresponsive */
    public Instant jobTimeoutLimit() { return clock.instant().minus(jobTimeout); }

    public BuildSystem buildSystem() { return buildSystem; }

    public DeploymentOrder deploymentOrder() { return order; }

    //--- Start of methods which triggers deployment jobs -------------------------

    /**
     * Called each time a job completes (successfully or not) to cause triggering of one or more follow-up jobs
     * (which may possibly the same job once over).
     *
     * @param report information about the job that just completed
     */
    public void triggerFromCompletion(JobReport report) {
        applications().lockOrThrow(report.applicationId(), application -> {
            application = application.withJobCompletion(report, clock.instant(), controller);

            // Handle successful starting and ending
            if (report.success()) {
                if (report.jobType() == JobType.component) {
                    if (acceptNewRevisionNow(application)) {
                        // Set this as the change we are doing, unless we are already pushing a platform change
                        if ( ! ( application.deploying().isPresent() &&
                                 (application.deploying().get() instanceof Change.VersionChange)))
                            application = application.withDeploying(Optional.of(Change.ApplicationChange.unknown()));
                    }
                    else { // postpone
                        applications().store(application.withOutstandingChange(true));
                        return;
                    }
                }
                else if (deploymentComplete(application)) {
                    // change completed
                    application = application.withDeploying(Optional.empty());
                }
            }

            // Trigger next
            if (report.success())
                application = trigger(order.nextAfter(report.jobType(), application), application,
                                      report.jobType().jobName() + " completed");
            else if (retryBecauseOutOfCapacity(application, report.jobType()))
                application = trigger(report.jobType(), application, true,
                                      "Retrying on out of capacity");
            else if (retryBecauseNewFailure(application, report.jobType()))
                application = trigger(report.jobType(), application, false,
                                      "Immediate retry on failure");

            applications().store(application);
        });
    }

    /** Returns whether all production zones listed in deployment spec has this change (or a newer version, if upgrade) */
    private boolean deploymentComplete(LockedApplication application) {
        if ( ! application.deploying().isPresent()) return true;
        Change change = application.deploying().get();

        for (JobType job : order.jobsFrom(application.deploymentSpec())) {
            if ( ! job.isProduction()) continue;

            Optional<ZoneId> zone = job.zone(this.controller.system());
            if ( ! zone.isPresent()) continue;

            Deployment deployment = application.deployments().get(zone.get());
            if (deployment == null) return false;

            // Check actual job outcome (the deployment)
            if (change instanceof VersionChange) {
                if (((VersionChange)change).version().isAfter(deployment.version())) return false; // later is ok
            }
            else if (((Change.ApplicationChange)change).revision().isPresent()) {
                if ( ! ((Change.ApplicationChange)change).revision().get().equals(deployment.revision())) return false;
            }
            else {
                return false; // If we don't yet know the revision we are changing to, then we are not complete
            }

        }

        return true;
    }

    /**
     * Find jobs that can and should run but are currently not.
     */
    public void triggerReadyJobs() {
        ApplicationList applications = ApplicationList.from(applications().asList());
        applications = applications.notPullRequest();
        for (Application application : applications.asList())
            applications().lockIfPresent(application.id(), this::triggerReadyJobs);
    }

    /** Find the next step to trigger if any, and triggers it */
    public void triggerReadyJobs(LockedApplication application) {
        if ( ! application.deploying().isPresent()) return;
        List<JobType> jobs =  order.jobsFrom(application.deploymentSpec());

        // Should the first step be triggered?
        if ( ! jobs.isEmpty() && jobs.get(0).equals(JobType.systemTest) ) {
            JobStatus systemTestStatus = application.deploymentJobs().jobStatus().get(JobType.systemTest);
            if (application.deploying().get() instanceof Change.VersionChange) {
                Version target = ((Change.VersionChange) application.deploying().get()).version();
                if (systemTestStatus == null
                    || ! systemTestStatus.lastTriggered().isPresent()
                    || ! systemTestStatus.isSuccess()
                    || ! systemTestStatus.lastTriggered().get().version().equals(target)
                    || systemTestStatus.isHanging(jobTimeoutLimit())) {
                    application = trigger(JobType.systemTest, application, false, "Upgrade to " + target);
                    controller.applications().store(application);
                }
            }
            else {
                JobStatus componentStatus = application.deploymentJobs().jobStatus().get(JobType.component);
                if (componentStatus != null && changesAvailable(application, componentStatus, systemTestStatus)) {
                    application = trigger(JobType.systemTest, application, false, "Available change in component");
                    controller.applications().store(application);
                }
            }
        }

        // Find next steps to trigger based on the state of the previous step
        for (JobType jobType : jobs) {
            JobStatus jobStatus = application.deploymentJobs().jobStatus().get(jobType);
            if (jobStatus == null) continue; // job has never run
            if (jobStatus.isRunning(jobTimeoutLimit())) continue;

            // Collect the subset of next jobs which have not run with the last changes
            List<JobType> nextToTrigger = new ArrayList<>();
            for (JobType nextJobType : order.nextAfter(jobType, application)) {
                JobStatus nextStatus = application.deploymentJobs().jobStatus().get(nextJobType);
                if (changesAvailable(application, jobStatus, nextStatus) || nextStatus.isHanging(jobTimeoutLimit()))
                    nextToTrigger.add(nextJobType);
            }
            // Trigger them in parallel
            application = trigger(nextToTrigger, application, "Available change in " + jobType.jobName());
            controller.applications().store(application);
        }
    }

    /**
     * Returns true if the previous job has completed successfully with a revision and/or version which is
     * newer (different) than the one last completed successfully in next
     */
    private boolean changesAvailable(Application application, JobStatus previous, JobStatus next) {
        if ( ! application.deploying().isPresent()) return false;
        if (next == null) return true;

        Change change = application.deploying().get();

        if (change instanceof Change.VersionChange) { // Propagate upgrade while making sure we never downgrade
            Version targetVersion = ((Change.VersionChange)change).version();

            if (next.type().isTest()) {
                // Is it this job's turn to upgrade?
                if ( previous != null && ! lastSuccessfulIs(targetVersion, previous.type(), application))
                    return false;

                // Has the upgrade already been done?
                if (lastSuccessfulIs(targetVersion, next.type(), application))
                    return false;
            }
            else if (next.type().isProduction()) {
                // Is the target version tested?
                if ( ! lastSuccessfulIs(targetVersion, JobType.stagingTest, application))
                    return false;

                // Is it this job's turn to upgrade?
                if ( previous.type().isProduction() && ! isOnAtLeastProductionVersion(targetVersion, application, previous.type()))
                    return false;

                // Is the next a production job for a zone already upgraded to this version or a newer one?
                if (isOnAtLeastProductionVersion(targetVersion, application, next.type()))
                    return false;
            }

            return true;
        }
        else { // revision changes do not need to handle downgrading
            if ( ! previous.lastSuccess().isPresent()) return false;
            if ( ! next.lastSuccess().isPresent()) return true;
            return previous.lastSuccess().get().revision().isPresent() &&
                   ! previous.lastSuccess().get().revision().equals(next.lastSuccess().get().revision());
        }
    }

    /**
     * Triggers a change of this application
     *
     * @param applicationId the application to trigger
     * @throws IllegalArgumentException if this application already have an ongoing change
     */
    public void triggerChange(ApplicationId applicationId, Change change) {
        applications().lockOrThrow(applicationId, application -> {
            if (application.deploying().isPresent()  && ! application.deploymentJobs().hasFailures())
                throw new IllegalArgumentException("Could not start " + change + " on " + application + ": " +
                                                   application.deploying().get() + " is already in progress");
            application = application.withDeploying(Optional.of(change));
            if (change instanceof Change.ApplicationChange)
                application = application.withOutstandingChange(false);
            application = trigger(JobType.systemTest, application, false,
                                  (change instanceof Change.VersionChange ? "Upgrading to " + ((Change.VersionChange)change).version() : "Deploying " + change));
            applications().store(application);
        });
    }

    /**
     * Cancels any ongoing upgrade of the given application
     *
     * @param applicationId the application to trigger
     */
    public void cancelChange(ApplicationId applicationId) {
        applications().lockOrThrow(applicationId, application -> {
            buildSystem.removeJobs(application.id());
            applications().store(application.withDeploying(Optional.empty()));
        });
    }

    //--- End of methods which triggers deployment jobs ----------------------------

    private ApplicationController applications() { return controller.applications(); }

    /** Retry immediately only if this job just started failing. Otherwise retry periodically */
    private boolean retryBecauseNewFailure(Application application, JobType jobType) {
        JobStatus jobStatus = application.deploymentJobs().jobStatus().get(jobType);
        return (jobStatus != null && jobStatus.firstFailing().get().at().isAfter(clock.instant().minus(Duration.ofSeconds(10))));
    }

    /** Decide whether to retry due to capacity restrictions */
    private boolean retryBecauseOutOfCapacity(Application application, JobType jobType) {
        JobStatus jobStatus = application.deploymentJobs().jobStatus().get(jobType);
        if (jobStatus == null || ! jobStatus.jobError().equals(Optional.of(JobError.outOfCapacity))) return false;
        // Retry the job if it failed recently
        return jobStatus.firstFailing().get().at().isAfter(clock.instant().minus(Duration.ofMinutes(15)));
    }

    /** Returns whether the given job type should be triggered according to deployment spec */
    private boolean hasJob(JobType jobType, Application application) {
        if ( ! jobType.isProduction()) return true; // Deployment spec only determines this for production jobs.
        return application.deploymentSpec().includes(jobType.environment(), jobType.region(controller.system()));
    }

    /**
     * Trigger a job for an application
     *
     * @param jobType the type of the job to trigger, or null to trigger nothing
     * @param application the application to trigger the job for
     * @param first whether to put the job at the front of the build system queue (or the back)
     * @param reason describes why the job is triggered
     * @return the application in the triggered state, which *must* be stored by the caller
     */
    private LockedApplication trigger(JobType jobType, LockedApplication application, boolean first, String reason) {
        if (jobType.isProduction() && isRunningProductionJob(application)) return application;
        return triggerAllowParallel(jobType, application, first, false, reason);
    }

    private LockedApplication trigger(List<JobType> jobs, LockedApplication application, String reason) {
        if (jobs.stream().anyMatch(JobType::isProduction) && isRunningProductionJob(application)) return application;
        for (JobType job : jobs)
            application = triggerAllowParallel(job, application, false, false, reason);
        return application;
    }

    /**
     * Trigger a job for an application, if allowed
     *
     * @param jobType the type of the job to trigger, or null to trigger nothing
     * @param application the application to trigger the job for
     * @param first whether to trigger the job before other jobs
     * @param force true to disable checks which should normally prevent this triggering from happening
     * @param reason describes why the job is triggered
     * @return the application in the triggered state, if actually triggered. This *must* be stored by the caller
     */
    public LockedApplication triggerAllowParallel(JobType jobType, LockedApplication application,
                                                  boolean first, boolean force, String reason) {
        if (jobType == null) return application; // we are passed null when the last job has been reached
        // Never allow untested changes to go through
        // Note that this may happen because a new change catches up and prevents an older one from continuing
        if ( ! application.deploymentJobs().isDeployableTo(jobType.environment(), application.deploying())) {
            log.warning(String.format("Want to trigger %s for %s with reason %s, but change is untested", jobType,
                                      application, reason));
            return application;
        }

        if ( ! force && ! allowedTriggering(jobType, application)) return application;
        log.info(String.format("Triggering %s for %s, %s: %s", jobType, application,
                               application.deploying().map(d -> "deploying " + d).orElse("restarted deployment"),
                               reason));
        buildSystem.addJob(application.id(), jobType, first);
        return application.withJobTriggering(jobType,
                                             application.deploying(),
                                             clock.instant(),
                                             application.deployVersionFor(jobType, controller),
                                             application.deployRevisionFor(jobType, controller),
                                             reason);
    }

    /** Returns true if the given proposed job triggering should be effected */
    private boolean allowedTriggering(JobType jobType, LockedApplication application) {
        // Note: We could make a more fine-grained and more correct determination about whether to block
        //       by instead basing the decision on what is currently deployed in the zone. However,
        //       this leads to some additional corner cases, and the possibility of blocking an application
        //       fix to a version upgrade, so not doing it now

        if (jobType.isProduction() && application.deploying().isPresent() &&
            application.deploying().get().blockedBy(application.deploymentSpec(), clock.instant())) return false;

        // Don't downgrade or redeploy the same version in production
        if (application.deploying().isPresent() && application.deploying().get() instanceof VersionChange &&
            jobType.isProduction() && isOnAtLeastProductionVersion(((VersionChange) application.deploying().get()).version(), application, jobType)) return false;

        if (application.deploymentJobs().isRunning(jobType, jobTimeoutLimit())) return false;
        if  ( ! hasJob(jobType, application)) return false;
        // Ignore applications that are not associated with a project
        if ( ! application.deploymentJobs().projectId().isPresent()) return false;

        return true;
    }

    private boolean isRunningProductionJob(Application application) {
        return JobList.from(application)
                .production()
                .running(jobTimeoutLimit())
                .anyMatch();
    }

    /**
     * Returns whether the current deployed version in the zone given by the job
     * is newer or equal to the given version. This may be the case even if the production job
     * in question failed, if the failure happens after deployment.
     * In that case we should never deploy an earlier version as that may potentially
     * downgrade production nodes which we are not guaranteed to support, and upgradibng to the current
     * version is just unnecessary work.
     */
    private boolean isOnAtLeastProductionVersion(Version version, Application application, JobType job) {
        if ( ! job.isProduction()) return false; // TODO: Throw if not prod, and change name/doc accordingly
        Optional<ZoneId> zone = job.zone(controller.system());
        if ( ! zone.isPresent()) return false;
        Deployment existingDeployment = application.deployments().get(zone.get());
        if (existingDeployment == null) return false;
        return existingDeployment.version().isAfter(version) || existingDeployment.version().equals(version);
    }

    private boolean acceptNewRevisionNow(LockedApplication application) {
        if ( ! application.deploying().isPresent()) return true;

        if (application.deploying().get() instanceof Change.ApplicationChange) return true; // more changes are ok

        if (application.deploymentJobs().hasFailures()) return true; // allow changes to fix upgrade problems

        if (application.isBlocked(clock.instant())) return true; // allow testing changes while upgrade blocked (debatable)

        // Otherwise, the application is currently upgrading, without failures, and we should wait with the revision.
        return false;
    }

    private boolean lastSuccessfulIs(Version version, JobType jobType, Application application) {
        JobStatus status = application.deploymentJobs().jobStatus().get(jobType);
        if (status == null) return false;
        Optional<JobStatus.JobRun> lastSuccessfulStagingRun = status.lastSuccess();
        if ( ! lastSuccessfulStagingRun.isPresent()) return false;
        return lastSuccessfulStagingRun.get().version().equals(version);
    }

}
