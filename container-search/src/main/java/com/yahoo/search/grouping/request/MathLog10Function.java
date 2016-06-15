// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.search.grouping.request;

import java.util.Arrays;

/**
 * @author balder
 */
public class MathLog10Function extends FunctionNode {
    /**
     * Constructs a new instance of this class.
     *
     * @param exp The expression to evaluate, double value will be requested.
     */
    public MathLog10Function(GroupingExpression exp) {
        super("math.log10", Arrays.asList(exp));
    }
}
