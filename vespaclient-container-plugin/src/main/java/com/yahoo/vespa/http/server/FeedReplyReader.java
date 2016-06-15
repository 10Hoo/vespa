// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.http.server;

import java.util.logging.Logger;

import com.yahoo.jdisc.Metric;
import com.yahoo.log.LogLevel;
import com.yahoo.messagebus.Reply;
import com.yahoo.messagebus.ReplyHandler;
import com.yahoo.messagebus.Trace;
import com.yahoo.vespa.http.client.core.ErrorCode;
import com.yahoo.vespa.http.client.core.OperationStatus;

/**
 * Catch message bus replies and make the available to a given session.
 *
 * @author <a href="mailto:steinar@yahoo-inc.com">Steinar Knutsen</a>
 */
public class FeedReplyReader implements ReplyHandler {

    private static final Logger log = Logger.getLogger(FeedReplyReader.class.getName());
    private final Metric metric;

    public FeedReplyReader(Metric metric) {
        this.metric = metric;
    }

    @Override
    public void handleReply(Reply reply) {
        Object o = reply.getContext();
        if (!(o instanceof ReplyContext)) {
            return;
        }
        ReplyContext context = (ReplyContext) o;
        metric.set(
                MetricNames.LATENCY,
                Double.valueOf((System.currentTimeMillis() - context.creationTime) / 1000.0d),
                null);
        if (reply.hasErrors()) {
            metric.add(MetricNames.FAILED, 1, null);
            enqueue(context, reply.getError(0).getMessage(), ErrorCode.ERROR, reply.getTrace());
        } else {
            metric.add(MetricNames.SUCCEEDED, 1, null);
            enqueue(context, "Document processed.", ErrorCode.OK, reply.getTrace());
        }
    }

    private void enqueue(ReplyContext context, String message, ErrorCode status, Trace trace) {
        try {
            String traceMessage = (trace != null && trace.getLevel() > 0) ? trace.toString() : "";
            context.feedReplies.put(new OperationStatus(message,
                    context.docId.toString(), status, traceMessage));
        } catch (InterruptedException e) {
            log.log(LogLevel.WARNING,
                    "Interrupted while enqueueing result from putting document with id: "
                            + context.docId.toString());
            Thread.currentThread().interrupt();
        }
    }

}