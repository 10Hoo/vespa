// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.benchmark;

import java.util.Map;

/**
 * Tester interface for loadable test runners.
 */
public interface Tester {
    public void subscribe();
    public boolean fetch();
    public boolean verify(Map<String, Map<String, String>> expected, long generation);
    public void close();
}
