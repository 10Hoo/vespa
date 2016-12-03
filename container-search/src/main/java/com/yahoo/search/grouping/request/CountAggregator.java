// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.search.grouping.request;

/**
 * This class represents an count-aggregator in a {@link GroupingExpression}. It evaluates to the number of labels
 * there are in the input.
 *
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public class CountAggregator extends AggregatorNode {

    /**
     * Constructs a new instance of this class.
     */
    public CountAggregator() {
        super("count");
    }
}
