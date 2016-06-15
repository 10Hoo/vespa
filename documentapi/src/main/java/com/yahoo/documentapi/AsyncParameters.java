// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.documentapi;

/**
 * Parameters for creating an async session
 *
 * @author <a href="mailto:bratseth@yahoo-inc.com">Jon Bratseth</a>
 */
public class AsyncParameters extends Parameters {

    private ResponseHandler responseHandler = null;

    public ResponseHandler getResponseHandler() {
        return responseHandler;
    }

    public AsyncParameters setResponseHandler(ResponseHandler responseHandler) {
        this.responseHandler = responseHandler;
        return this;
    }
}
