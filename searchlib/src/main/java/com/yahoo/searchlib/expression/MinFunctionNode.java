// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchlib.expression;

/**
 * This function is an instruction to return the minimum value of all its arguments.
 *
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public class MinFunctionNode extends NumericFunctionNode {

    public static final int classId = registerClass(0x4000 + 65, MinFunctionNode.class);

    @Override
    protected int onGetClassId() {
        return classId;
    }

    @Override
    protected void onArgument(final ResultNode arg, ResultNode result) {
        ((NumericResultNode)result).min(arg);
    }
}
