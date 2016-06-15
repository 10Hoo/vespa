// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.logserver.filter;

import com.yahoo.log.LogMessage;

/**
 * This filter is the complement of MetricsFilter
 *
 * @author  <a href="mailto:borud@yahoo-inc.com">Bjorn Borud</a>
 */
public class NoMetricsFilter implements LogFilter {
    final MetricsFilter filter = new MetricsFilter();

    public boolean isLoggable (LogMessage msg) {
        return (! filter.isLoggable(msg));
    }

    public String description () {
        return "Matches all log messages except Count and Value events";
    }
}
