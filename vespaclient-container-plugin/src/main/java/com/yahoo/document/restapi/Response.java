// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.restapi;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.yahoo.container.jdisc.HttpResponse;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Optional;

public class Response extends HttpResponse {

    private final static ObjectMapper objectMapper = new ObjectMapper();
    private final String jsonMessage;

    public Response(int code, Optional<ObjectNode> element, Optional<RestUri> restPath) {
        super(code);
        ObjectNode objectNode = element.orElse(objectMapper.createObjectNode());
        if (restPath.isPresent()) {
            objectNode.put("id", restPath.get().generateFullId());
            objectNode.put("pathId", restPath.get().getRawPath());
        }
        jsonMessage = objectNode.toString();
    }

    public static Response createErrorResponse(int code, String errorMessage) {
        ObjectNode objectNode = objectMapper.createObjectNode();
        objectNode.putArray("errors").add(errorMessage);
        return new Response(code, Optional.of(objectNode), Optional.<RestUri>empty());
    }

    public static Response createErrorResponse(int code, String errorMessage, RestUri restUri) {
        ObjectNode objectNode = objectMapper.createObjectNode();
        objectNode.putArray("errors").add(errorMessage);
        return new Response(code, Optional.of(objectNode), Optional.of(restUri));
    }

    @Override
    public void render(OutputStream stream) throws IOException {
        stream.write(jsonMessage.getBytes(StandardCharsets.UTF_8));
    }

    @Override
    public String getContentType() { return "application/json"; }

}