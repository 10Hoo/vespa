// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.search.grouping.request;

import java.util.Arrays;

/**
 * This class represents a todouble-function in a {@link GroupingExpression}. It converts the result of the argument to
 * a double. If the argument can not be converted, this function returns 0.
 *
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 */
public class ToDoubleFunction extends FunctionNode {

    /**
     * Constructs a new instance of this class.
     *
     * @param exp The expression to evaluate.
     */
    public ToDoubleFunction(GroupingExpression exp) {
        super("todouble", Arrays.asList(exp));
    }
}
