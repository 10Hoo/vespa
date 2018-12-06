// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.service.health;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.config.Registry;
import org.apache.http.config.RegistryBuilder;
import org.apache.http.conn.ConnectionKeepAliveStrategy;
import org.apache.http.conn.HttpClientConnectionManager;
import org.apache.http.conn.socket.ConnectionSocketFactory;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.DefaultConnectionKeepAliveStrategy;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.impl.conn.BasicHttpClientConnectionManager;
import org.apache.http.protocol.HttpContext;
import org.apache.http.util.EntityUtils;

import java.util.function.Function;

import static com.yahoo.yolean.Exceptions.uncheck;

/**
 * Health client
 *
 * NOT thread-safe.
 *
 * @author hakon
 */
public class HealthClient implements AutoCloseable {
    private static final ObjectMapper mapper = new ObjectMapper();
    private static final long MAX_CONTENT_LENGTH = 1L << 20; // 1 MB
    private static final int DEFAULT_TIMEOUT_MILLIS = 1_000;

    private static final ConnectionKeepAliveStrategy KEEP_ALIVE_STRATEGY =
            new DefaultConnectionKeepAliveStrategy() {
                @Override
                public long getKeepAliveDuration(HttpResponse response, HttpContext context) {
                    long keepAlive = super.getKeepAliveDuration(response, context);
                    if (keepAlive == -1) {
                        // Keep connections alive 60 seconds if a keep-alive value
                        // has not be explicitly set by the server
                        keepAlive = 60000;
                    }
                    return keepAlive;
                }
            };

    private final HealthEndpoint endpoint;
    private final CloseableHttpClient httpClient;
    private final Function<HttpEntity, String> getContentFunction;

    public HealthClient(HealthEndpoint endpoint) {
        this(endpoint,
                makeCloseableHttpClient(endpoint),
                entity -> uncheck(() -> EntityUtils.toString(entity)));
    }

    /** For testing. */
    HealthClient(HealthEndpoint endpoint,
                 CloseableHttpClient httpClient,
                 Function<HttpEntity, String> getContentFunction) {
        this.endpoint = endpoint;
        this.httpClient = httpClient;
        this.getContentFunction = getContentFunction;
    }

    public HealthEndpoint getEndpoint() {
        return endpoint;
    }

    public HealthInfo getHealthInfo() {
        try {
            return probeHealth();
        } catch (Exception e) {
            return HealthInfo.fromException(e);
        }
    }

    @Override
    public void close() {
        try {
            httpClient.close();
        } catch (Exception e) {
            // ignore
        }
    }

    private static CloseableHttpClient makeCloseableHttpClient(HealthEndpoint endpoint) {
        Registry<ConnectionSocketFactory> registry = RegistryBuilder.<ConnectionSocketFactory>create()
                .register(endpoint.getStateV1HealthUrl().getProtocol(), endpoint.getConnectionSocketFactory())
                .build();

        HttpClientConnectionManager connectionManager = new BasicHttpClientConnectionManager(registry);

        RequestConfig requestConfig = RequestConfig.custom()
                .setConnectTimeout(DEFAULT_TIMEOUT_MILLIS) // establishment of connection
                .setConnectionRequestTimeout(DEFAULT_TIMEOUT_MILLIS)  // connection from connection manager
                .setSocketTimeout(DEFAULT_TIMEOUT_MILLIS) // waiting for data
                .build();

        return HttpClients.custom()
                .setKeepAliveStrategy(KEEP_ALIVE_STRATEGY)
                .setConnectionManager(connectionManager)
                .disableAutomaticRetries()
                .setDefaultRequestConfig(requestConfig)
                .build();
    }

    private HealthInfo probeHealth() throws Exception {
        HttpGet httpget = new HttpGet(endpoint.getStateV1HealthUrl().toString());

        CloseableHttpClient httpClient = this.httpClient;
        if (httpClient == null) {
            throw new IllegalStateException("HTTP client never started or has closed");
        }

        try (CloseableHttpResponse httpResponse = httpClient.execute(httpget)) {
            int httpStatusCode = httpResponse.getStatusLine().getStatusCode();
            if (httpStatusCode < 200 || httpStatusCode >= 300) {
                return HealthInfo.fromBadHttpStatusCode(httpStatusCode);
            }

            HttpEntity bodyEntity = httpResponse.getEntity();
            long contentLength = bodyEntity.getContentLength();
            if (contentLength > MAX_CONTENT_LENGTH) {
                throw new IllegalArgumentException("Content too long: " + contentLength + " bytes");
            }
            String body = getContentFunction.apply(bodyEntity);
            HealthResponse healthResponse = mapper.readValue(body, HealthResponse.class);

            if (healthResponse.status == null || healthResponse.status.code == null) {
                return HealthInfo.fromHealthStatusCode(HealthResponse.Status.DEFAULT_STATUS);
            } else {
                return HealthInfo.fromHealthStatusCode(healthResponse.status.code);
            }
        }
    }
}
