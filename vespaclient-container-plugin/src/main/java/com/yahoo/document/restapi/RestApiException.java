// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.restapi;

/**
 * Exceptions for Rest API
 *
 * @author dybdahl
 */
public class RestApiException extends Exception {

    private final Response response;

    public RestApiException(Response response) {
        this.response = response;
    }

    public Response getResponse() {
        return response;
    }

}
