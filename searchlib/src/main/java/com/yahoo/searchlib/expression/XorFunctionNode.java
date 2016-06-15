// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchlib.expression;

/**
 * This function is an instruction to perform bitwise XOR on the result of all arguments in order.
 *
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public class XorFunctionNode extends BitFunctionNode {

    public static final int classId = registerClass(0x4000 + 69, XorFunctionNode.class);

    @Override
    protected int onGetClassId() {
        return classId;
    }

    public void onArgument(final ResultNode arg, IntegerResultNode result) {
        result.xorOp(arg);
    }
}
