// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchlib.expression;

/**
 * This is an abstract super-class for all non-unary functions that operator on bit values.
 *
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public abstract class BitFunctionNode extends NumericFunctionNode {

    public static final int classId = registerClass(0x4000 + 47, BitFunctionNode.class);

    @Override
    protected int onGetClassId() {
        return classId;
    }

    @Override
    protected void onArgument(final ResultNode arg, ResultNode result) {
        onArgument(arg, (IntegerResultNode)result);
    }

    @Override
    protected void onPrepareResult() {
        setResult(new IntegerResultNode(0));
    }

    /**
     * Method for performing onArgument on integers, the only type supported for bit operations.
     *
     * @param arg    Argument given to the bit function.
     * @param result Place to store the result.
     */
    protected abstract void onArgument(final ResultNode arg, IntegerResultNode result);
}
