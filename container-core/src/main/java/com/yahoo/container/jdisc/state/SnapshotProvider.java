// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.container.jdisc.state;

import java.io.PrintStream;

/**
 * An interface for components supplying a state snapshot where persistence and
 * other pre-processing has been done.
 *
 * @author <a href="mailto:steinar@yahoo-inc.com">Steinar Knutsen</a>
 */
public interface SnapshotProvider {
    public MetricSnapshot latestSnapshot();
    public void histogram(PrintStream output);
}
