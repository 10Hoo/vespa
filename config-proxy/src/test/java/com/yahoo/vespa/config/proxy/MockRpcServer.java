// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.proxy;

import com.yahoo.vespa.config.RawConfig;
import com.yahoo.vespa.config.protocol.JRTServerConfigRequest;

/**
 * @author musum
 */
public class MockRpcServer implements RpcServer {

    volatile long responses = 0;
    volatile long errorResponses = 0;

    public void returnOkResponse(JRTServerConfigRequest request, RawConfig config) {
        responses++;
    }

    public void returnErrorResponse(JRTServerConfigRequest request, int errorCode, String message) {
        responses++;
        errorResponses++;
    }
}
