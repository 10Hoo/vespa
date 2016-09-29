package com.yahoo.vespa.hosted.dockerapi.metrics;

import com.yahoo.metrics.simple.Counter;

/**
 * Forwards sample to {@link com.yahoo.metrics.simple.Counter} to be displayed in /state/v1/metrics,
 * while also saving the value so it can be accessed programatically later.
 *
 * @author valerijf
 */
public class CounterWrapper implements MetricValue {
    private final Counter counter;
    private long value = 0;

    CounterWrapper(Counter counter) {
        this.counter = counter;
    }

    public void add() {
        add(1L);
    }

    public void add(long n) {
        synchronized (counter) {
            counter.add(n);
            value += n;
        }
    }

    @Override
    public Number getValue() {
        synchronized (counter) {
            return value;
        }
    }
}