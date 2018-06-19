// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.api.integration;

import com.yahoo.config.provision.ApplicationId;

/**
 * @author jonmv
 */
public interface BuildService {

    /**
     * Enqueues a job defined by buildJob in an external build system.
     *
     * Implementations should throw an exception if the triggering fails.
     */
    void trigger(BuildJob buildJob);

    /**
     * Returns the state of the given job in the build service.
     */
    JobState stateOf(BuildJob buildJob);

    /**
     * Returns whether the given build job should be performed by this build service.
     */
    default boolean builds(BuildJob buildJob) { return true; }

    enum JobState {

        /** Job is not running, and may be triggered. */
        idle,

        /** Job is already running, and will be queued if triggered now. */
        running,

        /** Job is running and queued and will automatically be started again after it finishes its current run. */
        queued,

        /** Job is disabled, i.e., it can not be triggered. */
        disabled

    }


    class BuildJob {

        private final ApplicationId applicationId;
        private final long projectId;
        private final String jobName;

        protected BuildJob(ApplicationId applicationId, long projectId, String jobName) {
            this.applicationId = applicationId;
            this.projectId = projectId;
            this.jobName = jobName;
        }

        public static BuildJob of(ApplicationId applicationId, long projectId, String jobName) {
            return new BuildJob(applicationId, projectId, jobName);
        }

        public ApplicationId applicationId() { return applicationId; }
        public long projectId() { return projectId; }
        public String jobName() { return jobName; }

        @Override
        public final boolean equals(Object o) {
            if (this == o) return true;
            if ( ! (o instanceof BuildJob)) return false;

            BuildJob buildJob = (BuildJob) o;

            if (projectId != buildJob.projectId) return false;
            return jobName.equals(buildJob.jobName);
        }

        @Override
        public final int hashCode() {
            int result = (int) (projectId ^ (projectId >>> 32));
            result = 31 * result + jobName.hashCode();
            return result;
        }

        @Override
        public String toString() {
            return jobName + " for " + applicationId + " with project " + projectId;
        }

    }

}
