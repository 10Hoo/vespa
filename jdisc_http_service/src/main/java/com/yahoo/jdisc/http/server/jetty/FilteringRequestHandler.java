// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.jdisc.http.server.jetty;

import com.google.common.base.Preconditions;
import com.yahoo.jdisc.Request;
import com.yahoo.jdisc.Response;
import com.yahoo.jdisc.application.BindingSet;
import com.yahoo.jdisc.handler.AbstractRequestHandler;
import com.yahoo.jdisc.handler.BindingNotFoundException;
import com.yahoo.jdisc.handler.CompletionHandler;
import com.yahoo.jdisc.handler.ContentChannel;
import com.yahoo.jdisc.handler.RequestDeniedException;
import com.yahoo.jdisc.handler.RequestHandler;
import com.yahoo.jdisc.handler.ResponseHandler;
import com.yahoo.jdisc.http.HttpRequest;
import com.yahoo.jdisc.http.core.CompletionHandlers;
import com.yahoo.jdisc.http.filter.RequestFilter;
import com.yahoo.jdisc.http.filter.ResponseFilter;

import java.nio.ByteBuffer;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Request handler that invokes request and response filters in addition to the bound request handler.
 *
 * @author bakksjo
 * $Id$
 */
class FilteringRequestHandler extends AbstractRequestHandler {
    private static final ContentChannel COMPLETING_CONTENT_CHANNEL = new ContentChannel() {
        @Override
        public void write(final ByteBuffer buf, final CompletionHandler handler) {
            CompletionHandlers.tryComplete(handler);
        }

        @Override
        public void close(final CompletionHandler handler) {
            CompletionHandlers.tryComplete(handler);
        }
    };

    private final BindingSet<RequestFilter> requestFilters;
    private final BindingSet<ResponseFilter> responseFilters;

    public FilteringRequestHandler(
            final BindingSet<RequestFilter> requestFilters,
            final BindingSet<ResponseFilter> responseFilters) {
        this.requestFilters = requestFilters;
        this.responseFilters = responseFilters;
    }

    @Override
    public ContentChannel handleRequest(final Request request, final ResponseHandler originalResponseHandler) {
        Preconditions.checkArgument(request instanceof HttpRequest, "Expected HttpRequest, got " + request);
        Objects.requireNonNull(originalResponseHandler, "responseHandler");

        final RequestFilter requestFilter = requestFilters.resolve(request.getUri());
        final ResponseFilter responseFilter = responseFilters.resolve(request.getUri());
        // Not using request.connect() here - it adds logic for error handling that we'd rather leave to the framework.
        final RequestHandler resolvedRequestHandler = request.container().resolveHandler(request);

        if (resolvedRequestHandler == null) {
            throw new BindingNotFoundException(request.getUri());
        }

        final RequestHandler requestHandler = new ReferenceCountingRequestHandler(resolvedRequestHandler);

        final ResponseHandler responseHandler;
        if (responseFilter != null) {
            responseHandler = new FilteringResponseHandler(originalResponseHandler, responseFilter, request);
        } else {
            responseHandler = originalResponseHandler;
        }

        if (requestFilter != null) {
            final InterceptingResponseHandler interceptingResponseHandler
                    = new InterceptingResponseHandler(responseHandler);
            requestFilter.filter(HttpRequest.class.cast(request), interceptingResponseHandler);
            if (interceptingResponseHandler.hasProducedResponse()) {
                return COMPLETING_CONTENT_CHANNEL;
            }
        }

        final ContentChannel contentChannel = requestHandler.handleRequest(request, responseHandler);
        if (contentChannel == null) {
            throw new RequestDeniedException(request);
        }
        return contentChannel;
    }

    private static class FilteringResponseHandler implements ResponseHandler {
        private final ResponseHandler delegate;
        private final ResponseFilter responseFilter;
        private final Request request;

        public FilteringResponseHandler(
                final ResponseHandler delegate,
                final ResponseFilter responseFilter,
                final Request request) {
            this.delegate = Objects.requireNonNull(delegate);
            this.responseFilter = Objects.requireNonNull(responseFilter);
            this.request = request;
        }

        @Override
        public ContentChannel handleResponse(final Response response) {
            responseFilter.filter(response, request);
            return delegate.handleResponse(response);
        }
    }

    private static class InterceptingResponseHandler implements ResponseHandler {
        private final ResponseHandler delegate;
        private AtomicBoolean hasResponded = new AtomicBoolean(false);

        public InterceptingResponseHandler(final ResponseHandler delegate) {
            this.delegate = Objects.requireNonNull(delegate);
        }

        @Override
        public ContentChannel handleResponse(final Response response) {
            final ContentChannel content = delegate.handleResponse(response);
            hasResponded.set(true);
            return content;
        }

        public boolean hasProducedResponse() {
            return hasResponded.get();
        }
    }
}
