// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.orchestrator;

import com.yahoo.vespa.applicationmodel.ApplicationInstanceReference;
import com.yahoo.vespa.applicationmodel.ApplicationInstance;
import com.yahoo.vespa.applicationmodel.HostName;
import com.yahoo.vespa.service.monitor.ServiceMonitorStatus;

import java.util.Optional;
import java.util.Set;

/**
 * @author oyving
 */
public interface InstanceLookupService {
    Optional<ApplicationInstance<ServiceMonitorStatus>> findInstanceById(ApplicationInstanceReference applicationInstanceReference);
    Optional<ApplicationInstance<ServiceMonitorStatus>> findInstanceByHost(HostName hostName);
    Set<ApplicationInstanceReference> knownInstances();
}
