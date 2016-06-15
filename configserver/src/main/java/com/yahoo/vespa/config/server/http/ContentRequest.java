// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.server.http;

import com.yahoo.config.application.api.ApplicationFile;
import com.yahoo.container.jdisc.HttpRequest;
import com.yahoo.path.Path;
import com.yahoo.vespa.config.server.session.LocalSession;

import java.io.InputStream;

/**
 * Represents a {@link ContentRequest}, and contains common functionality for content requests for all content handlers.
 *
 * @author lulf
 * @since 5.3
 */
public abstract class ContentRequest {
    private static final String RETURN_QUERY_PROPERTY = "return";

    enum ReturnType {CONTENT, STATUS}

    private final long sessionId;
    private final String path;
    private final ApplicationFile file;
    private final HttpRequest request;

    protected ContentRequest(HttpRequest request, LocalSession session) {
        this.request = request;
        this.sessionId = session.getSessionId();
        this.path = getContentPath(request);
        this.file = session.getApplicationFile(Path.fromString(path), getApplicationFileMode(request.getMethod()));
    }

    private LocalSession.Mode getApplicationFileMode(com.yahoo.jdisc.http.HttpRequest.Method method) {
        switch (method) {
            case GET:
            case OPTIONS:
                return LocalSession.Mode.READ;
            default:
                return LocalSession.Mode.WRITE;
        }
    }

    ReturnType getReturnType() {
        if (request.hasProperty(RETURN_QUERY_PROPERTY)) {
            String type = request.getProperty(RETURN_QUERY_PROPERTY);
            switch (type) {
                case "content":
                return ReturnType.CONTENT;
                case "status":
                return ReturnType.STATUS;
                default:
                throw new BadRequestException("return=" + type + " is an illegal argument. Only " +
                    ReturnType.CONTENT.name() + " and " + ReturnType.STATUS.name() + " are allowed");
            }
        } else {
            return ReturnType.CONTENT;
        }
    }

    protected abstract String getPathPrefix();
    protected abstract String getContentPath(HttpRequest request);

    String getUrlBase(String appendStr) {
        return Utils.getUrlBase(request, getPathPrefix() + appendStr);
    }

    boolean isRecursive() {
        return request.getBooleanProperty("recursive");
    }

    boolean hasRequestBody() {
        return request.getData() != null;
    }

    InputStream getData() {
        return request.getData();
    }


    String getPath() {
        return path;
    }

    ApplicationFile getFile() {
        return file;
    }

    ApplicationFile getExistingFile() {
        if (!file.exists()) {
            throw new NotFoundException("Session " + sessionId + " does not contain a file '" + path + "'");
        }
        return file;
    }
}
