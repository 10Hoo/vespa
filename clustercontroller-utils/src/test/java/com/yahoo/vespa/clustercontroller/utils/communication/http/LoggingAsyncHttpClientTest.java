// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.clustercontroller.utils.communication.http;

import com.yahoo.vespa.clustercontroller.utils.communication.async.AsyncOperation;
import com.yahoo.vespa.clustercontroller.utils.communication.async.AsyncOperationImpl;
import junit.framework.TestCase;

import java.util.logging.Level;
import java.util.logging.Logger;

public class LoggingAsyncHttpClientTest extends TestCase {
    class HttpClient implements AsyncHttpClient<HttpResult> {
        AsyncOperationImpl<HttpResult> lastOp;
        @Override
        public AsyncOperation<HttpResult> execute(HttpRequest r) {
            return lastOp = new AsyncOperationImpl<>("test");
        }
        @Override
        public void close() {
        }
    }

    public void testWithoutDebugLog() throws Exception {
        doRequests();
    }

    public void testWithDebugLog() throws Exception {
        Logger log = Logger.getLogger(LoggingAsyncHttpClient.class.getName());
        log.setLevel(Level.FINE);
        doRequests();
    }

    private void doRequests() {
        {
            HttpClient client = new HttpClient();
            LoggingAsyncHttpClient<HttpResult> loggingClient = new LoggingAsyncHttpClient<>(client);
            AsyncOperation<HttpResult> op = loggingClient.execute(new HttpRequest());
            client.lastOp.setResult(new HttpResult().setContent("foo"));
            assertEquals("foo", op.getResult().getContent());
        }
        {
            HttpClient client = new HttpClient();
            LoggingAsyncHttpClient<HttpResult> loggingClient = new LoggingAsyncHttpClient<>(client);
            AsyncOperation<HttpResult> op = loggingClient.execute(new HttpRequest());
            client.lastOp.setFailure(new Exception("foo"));
            assertEquals("foo", op.getCause().getMessage());
        }

    }
}
