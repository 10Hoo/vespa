// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.container.jdisc.state;

import com.yahoo.jdisc.Metric;
import com.yahoo.jdisc.application.MetricConsumer;

import java.util.Map;

/**
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen Hult</a>
 */
final class StateMetricConsumer implements MetricConsumer {

    final static Metric.Context NULL_CONTEXT = StateMetricContext.newInstance(null);
    private final Object lock = new Object();
    private MetricSnapshot metricSnapshot = new MetricSnapshot();

    @Override
    public void set(String key, Number val, Metric.Context ctx) {
        synchronized (lock) {
            metricSnapshot.set(dimensionsOrDefault(ctx), key, val);
        }
    }

    private MetricDimensions dimensionsOrDefault(Metric.Context ctx) {
        return (MetricDimensions)(ctx != null ? ctx : NULL_CONTEXT);
    }

    @Override
    public void add(String key, Number val, Metric.Context ctx) {
        synchronized (lock) {
            metricSnapshot.add(dimensionsOrDefault(ctx), key, val);
        }
    }

    @Override
    public Metric.Context createContext(Map<String, ?> properties) {
        return StateMetricContext.newInstance(properties);
    }

    MetricSnapshot createSnapshot() {
        MetricSnapshot metricSnapshot;
        synchronized (lock) {
            metricSnapshot = this.metricSnapshot;
            this.metricSnapshot = this.metricSnapshot.createSnapshot();
        }
        return metricSnapshot;
    }
}
