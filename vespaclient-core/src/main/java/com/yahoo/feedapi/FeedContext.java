// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.feedapi;

import com.yahoo.cloud.config.ClusterListConfig;
import com.yahoo.cloud.config.SlobroksConfig;
import com.yahoo.document.config.DocumentmanagerConfig;
import com.yahoo.jdisc.Metric;
import com.yahoo.vespa.config.content.LoadTypeConfig;
import com.yahoo.document.DocumentTypeManager;
import com.yahoo.clientmetrics.ClientMetrics;
import com.yahoo.vespaclient.ClusterList;
import com.yahoo.vespaclient.config.FeederConfig;

import java.util.Map;
import java.util.TreeMap;

public class FeedContext {
    
    private final SessionFactory factory;
    private final MessagePropertyProcessor propertyProcessor;
    private final DocumentTypeManager docTypeManager;
    private final ClusterList clusterList;
    private final ClientMetrics metrics;
    private final Metric metric;
    private Map<String, SharedSender> senders = new TreeMap<>();

    public static final Object sync = new Object();
    public static FeedContext instance = null;

    public FeedContext(MessagePropertyProcessor propertyProcessor, SessionFactory factory, DocumentTypeManager manager, ClusterList clusterList, Metric metric) {
        this.propertyProcessor = propertyProcessor;
        this.factory = factory;
        docTypeManager = manager;
        this.clusterList = clusterList;
        metrics = new ClientMetrics();
        this.metric = metric;
    }

    public ClientMetrics getMetrics() {
        return metrics;
    }

    public ClusterList getClusterList() {
        return clusterList;
    }

    public SessionFactory getSessionFactory() {
        return factory;
    }

    public void shutdownSenders() {
        for (SharedSender s : senders.values()) {
            s.shutdown();
        }
    }

    public synchronized SharedSender getSharedSender(String route) {
        if (propertyProcessor.configChanged()) {
            Map<String, SharedSender> newSenders = new TreeMap<>();

            for (Map.Entry<String, SharedSender> sender : senders.entrySet()) {
                newSenders.put(sender.getKey(), new SharedSender(sender.getKey(), factory, sender.getValue(), metric));
            }

            shutdownSenders();
            senders = newSenders;
            propertyProcessor.setConfigChanged(false);
        }

        if (route == null) {
            route = propertyProcessor.getFeederOptions().getRoute();
        }

        SharedSender sender = senders.get(route);

        if (sender == null) {
            sender = new SharedSender(route, factory, sender, metric);
            senders.put(route, sender);
            metrics.addRouteMetricSet(sender.getMetrics());
        }

        return sender;
    }

    public MessagePropertyProcessor getPropertyProcessor() {
        return propertyProcessor;
    }

    public DocumentTypeManager getDocumentTypeManager() {
        return docTypeManager;
    }

    public static FeedContext getInstance(FeederConfig feederConfig, 
                                          LoadTypeConfig loadTypeConfig, 
                                          DocumentmanagerConfig documentmanagerConfig, 
                                          SlobroksConfig slobroksConfig,
                                          ClusterListConfig clusterListConfig,
                                          Metric metric) {
        synchronized (sync) {
            try {
                if (instance == null) {
                    MessagePropertyProcessor proc = new MessagePropertyProcessor(feederConfig, loadTypeConfig);
                    MessageBusSessionFactory mbusFactory = new MessageBusSessionFactory(proc, documentmanagerConfig, slobroksConfig);
                    instance = new FeedContext(proc,
                                               mbusFactory,
                                               mbusFactory.getAccess().getDocumentTypeManager(),
                                               new ClusterList(clusterListConfig), metric);
                } else {
                    instance.getPropertyProcessor().configure(feederConfig, loadTypeConfig);
                }

                return instance;
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }

}
