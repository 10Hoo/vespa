// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config;

/**
 * Interface for counters.
 *
 * @author lulf
 * @since 5.9
 */
public interface GenerationCounter {
    /**
     * Increment counter and return new value.
     *
     * @return incremented counter value.
     */
    public long increment();

    /**
     * @return current counter value.
     */
    public long get();
}
