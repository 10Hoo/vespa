// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.proxy;

import com.yahoo.concurrent.DaemonThreadFactory;
import com.yahoo.config.subscription.ConfigSourceSet;
import com.yahoo.config.subscription.impl.JRTConfigRequester;
import com.yahoo.jrt.Request;
import com.yahoo.jrt.Spec;
import com.yahoo.jrt.Supervisor;
import com.yahoo.jrt.Target;
import com.yahoo.jrt.Transport;
import com.yahoo.log.LogLevel;
import com.yahoo.vespa.config.*;
import com.yahoo.vespa.config.protocol.JRTServerConfigRequest;

import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Logger;

/**
 * An Rpc client to a config source
 *
 * @author hmusum
 * @since 5.1.9
 */
public class RpcConfigSourceClient extends ConfigSourceClient {

    private final static Logger log = Logger.getLogger(RpcConfigSourceClient.class.getName());
    private final Supervisor supervisor = new Supervisor(new Transport());

    private final ConfigSourceSet configSourceSet;
    private final HashMap<ConfigCacheKey, Subscriber> activeSubscribers = new HashMap<>();
    private final Object activeSubscribersLock = new Object();
    private final MemoryCache memoryCache;
    private final ClientUpdater clientUpdater;
    private final DelayedResponses delayedResponses;
    private final TimingValues timingValues;

    private ExecutorService exec;
    private Map<ConfigSourceSet, JRTConfigRequester> requesterPool;


    RpcConfigSourceClient(ConfigSourceSet configSourceSet,
                          ClientUpdater clientUpdater,
                          CacheManager cacheManager,
                          TimingValues timingValues,
                          DelayedResponses delayedResponses) {
        this.configSourceSet = configSourceSet;
        this.clientUpdater = clientUpdater;
        this.memoryCache = cacheManager.getMemoryCache();
        this.delayedResponses = delayedResponses;
        this.timingValues = timingValues;
        checkConfigSources();
        exec = Executors.newCachedThreadPool(new DaemonThreadFactory());
        requesterPool = createRequesterPool(configSourceSet, timingValues);
    }

    /**
     * Creates a requester (connection) pool of one entry, to be used each time this {@link RpcConfigSourceClient} is used
     * @param ccs a {@link ConfigSourceSet}
     * @param timingValues a {@link TimingValues}
     * @return requester map
     */
    private Map<ConfigSourceSet, JRTConfigRequester> createRequesterPool(ConfigSourceSet ccs, TimingValues timingValues) {
        Map<ConfigSourceSet, JRTConfigRequester> ret = new HashMap<>();
        if (ccs.getSources().isEmpty()) return ret; // unit test, just skip creating any requester
        ret.put(ccs, JRTConfigRequester.get(new JRTConnectionPool(ccs), timingValues));
        return ret;
    }

    /**
     * Checks if config sources are available
     */
    private boolean checkConfigSources() {
        if (configSourceSet == null || configSourceSet.getSources() == null || configSourceSet.getSources().size() == 0) {
            log.log(LogLevel.WARNING, "No config sources defined, could not check connection");
            return false;
        } else {
            Request req = new Request("ping");
            for (String configSource : configSourceSet.getSources()) {
                Spec spec = new Spec(configSource);
                Target target = supervisor.connect(spec);
                target.invokeSync(req, 10.0);
                if (target.isValid()) {
                    log.log(LogLevel.DEBUG, "Created connection to config source at " + spec.toString());
                    return true;
                } else {
                    log.log(LogLevel.WARNING, "Could not connect to config source at " + spec.toString());
                }
                target.close();
            }
            String extra = "";
            log.log(LogLevel.WARNING, "Could not connect to any config source in set " + configSourceSet.toString() +
                    ", please make sure config server(s) are running. " + extra);
        }
        return false;
    }

    /**
     * Retrieves the requested config from the cache or the remote server.
     * <p>
     * If the requested config is different from the one in cache, the cached request is returned immediately.
     * If they are equal, this method returns null.
     * <p>
     * If the config was not in cache, this method starts a <em>Subscriber</em> in a separate thread
     * that gets the config and calls updateSubscribers().
     *
     * @param input The config to retrieve - can be empty (no payload), or have a valid payload.
     * @return A Config with a payload.
     */
    @Override
    RawConfig getConfig(RawConfig input, JRTServerConfigRequest request) {
        long start = System.currentTimeMillis();
        RawConfig ret = null;
        final ConfigCacheKey configCacheKey = new ConfigCacheKey(input.getKey(), input.getDefMd5());
        RawConfig cachedConfig = memoryCache.get(configCacheKey);
        boolean needToGetConfig = true;

        if (cachedConfig != null) {
            log.log(LogLevel.DEBUG, "Found config " + configCacheKey + " in cache, generation=" + cachedConfig.getGeneration() +
                    ",configmd5=" + cachedConfig.getConfigMd5());
            if (log.isLoggable(LogLevel.SPAM)) {
                log.log(LogLevel.SPAM, "input config=" + input + ",cached config=" + cachedConfig);
            }
            // equals() also takes generation into account
            if (!cachedConfig.equals(input)) {
                log.log(LogLevel.SPAM, "Cached config is not equal to requested, will return it");
                ret = cachedConfig;
            }
            if (!cachedConfig.isError()) {
                needToGetConfig = false;
            }
        }
        if (!ProxyServer.configOrGenerationHasChanged(ret, request)) {
            if (log.isLoggable(LogLevel.DEBUG)) {
                log.log(LogLevel.DEBUG, "Delaying response " + request.getShortDescription() + " (" +
                        (System.currentTimeMillis() - start) + " ms)");
            }
            delayedResponses.add(new DelayedResponse(request));
        }
        if (needToGetConfig) {
            synchronized (activeSubscribersLock) {
                if (activeSubscribers.containsKey(configCacheKey)) {
                    log.log(LogLevel.DEBUG, "Already a subscriber running for: " + configCacheKey);
                } else {
                    log.log(LogLevel.DEBUG, "Could not find good config in cache, creating subscriber for: " + configCacheKey);
                    Subscriber subscriber = new UpstreamConfigSubscriber(input, clientUpdater, configSourceSet, timingValues, requesterPool, activeSubscribers);
                    activeSubscribers.put(configCacheKey, subscriber);
                    exec.execute(subscriber);
                }
            }
        }
        return ret;
    }

    @Override
    void cancel() {
        shutdownSourceConnections();
    }

    /**
     * Takes down connection(s) to config sources and running tasks
     */
    @Override
    void shutdownSourceConnections() {
        synchronized (activeSubscribersLock) {
            for (Subscriber subscriber : activeSubscribers.values()) {
                subscriber.cancel();
            }
            activeSubscribers.clear();
        }
        exec.shutdown();
        for (JRTConfigRequester requester : requesterPool.values()) {
            requester.close();
        }
    }

    @Override
    String getActiveSourceConnection() {
        if (requesterPool.get(configSourceSet) != null) {
            return requesterPool.get(configSourceSet).getConnectionPool().getCurrent().getAddress();
        } else {
            return "";
        }
    }

    @Override
    List<String> getSourceConnections() {
        ArrayList<String> ret = new ArrayList<>();
        final JRTConfigRequester jrtConfigRequester = requesterPool.get(configSourceSet);
        if (jrtConfigRequester != null) {
            ret.addAll(configSourceSet.getSources());
        }
        return ret;
    }
}
