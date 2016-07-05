// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.orchestrator.resources;

import com.yahoo.container.jaxrs.annotation.Component;
import com.yahoo.log.LogLevel;
import com.yahoo.vespa.applicationmodel.HostName;
import com.yahoo.vespa.orchestrator.BatchHostNameNotFoundException;
import com.yahoo.vespa.orchestrator.BatchInternalErrorException;
import com.yahoo.vespa.orchestrator.Orchestrator;
import com.yahoo.vespa.orchestrator.policy.BatchHostStateChangeDeniedException;
import com.yahoo.vespa.orchestrator.restapi.HostSuspensionApi;
import com.yahoo.vespa.orchestrator.restapi.wire.BatchHostSuspendRequest;
import com.yahoo.vespa.orchestrator.restapi.wire.BatchOperationResult;

import javax.inject.Inject;
import javax.ws.rs.Path;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import java.util.List;
import java.util.logging.Logger;
import java.util.stream.Collectors;

@Path(HostSuspensionApi.PATH_PREFIX)
public class HostSuspensionResource implements HostSuspensionApi {
    private static final Logger log = Logger.getLogger(HostSuspensionResource.class.getName());

    private final Orchestrator orchestrator;

    @Inject
    public HostSuspensionResource(@Component Orchestrator orchestrator) {
        this.orchestrator = orchestrator;
    }

    @Override
    public BatchOperationResult suspendAll(BatchHostSuspendRequest request) throws WebApplicationException {
        final String ph = request.getParentHostname();
        if (ph == null || ph.isEmpty()) {
            String message = "parentHostname missing or empty in request: " + request;
            log.log(LogLevel.DEBUG, message);
            throw createWebApplicationException(message, Response.Status.BAD_REQUEST);
        }
        final List<String> h = request.getHostnames();
        if (h == null || h.isEmpty()) {
            String message = "hostnames missing in request: " + request;
            log.log(LogLevel.DEBUG, message);
            throw createWebApplicationException(message, Response.Status.BAD_REQUEST);
        }

        HostName parentHostname = new HostName(ph);
        List<HostName> hostNames = request.getHostnames().stream().map(HostName::new).collect(Collectors.toList());

        try {
            orchestrator.suspendAll(parentHostname, hostNames);
        } catch (BatchHostStateChangeDeniedException e) {
            log.log(LogLevel.DEBUG, "Failed to suspend nodes " + hostNames + " with parent host " + parentHostname, e);
            throw createWebApplicationException(e.getMessage(), Response.Status.CONFLICT);
        } catch (BatchHostNameNotFoundException e) {
            log.log(LogLevel.DEBUG, "Failed to suspend nodes " + hostNames + " with parent host " + parentHostname, e);
            // Note that we're returning BAD_REQUEST instead of NOT_FOUND because the resource identified
            // by the URL path was found. It's one of the hostnames in the request it failed to find.
            throw createWebApplicationException(e.getMessage(), Response.Status.BAD_REQUEST);
        } catch (BatchInternalErrorException e) {
            log.log(LogLevel.DEBUG, "Failed to suspend nodes " + hostNames + " with parent host " + parentHostname, e);
            throw createWebApplicationException(e.getMessage(), Response.Status.INTERNAL_SERVER_ERROR);
        }

        log.log(LogLevel.DEBUG, "Suspended " + hostNames + " with parent " + parentHostname);
        return BatchOperationResult.successResult();
    }

    private WebApplicationException createWebApplicationException(String errorMessage, Response.Status status) {
        return new WebApplicationException(
                Response.status(status)
                        .entity(new BatchOperationResult(errorMessage))
                        .type(MediaType.APPLICATION_JSON_TYPE)
                        .build());
    }
}
