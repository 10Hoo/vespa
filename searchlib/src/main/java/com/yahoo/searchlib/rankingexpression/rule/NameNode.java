// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchlib.rankingexpression.rule;

import com.yahoo.searchlib.rankingexpression.evaluation.Context;
import com.yahoo.searchlib.rankingexpression.evaluation.Value;

import java.util.Deque;

/**
 * An opaque name in a ranking expression. This is used to represent names passed to the context
 * and interpreted by the given context in a way which is opaque to the ranking expressions.
 *
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public final class NameNode extends ExpressionNode {

    private final String name;

    public NameNode(String name) {
        this.name = name;
    }

    public String getValue() {
        return name;
    }

    @Override
    public String toString(SerializationContext context, Deque<String> path, CompositeNode parent) {
        return name;
    }

    @Override
    public Value evaluate(Context context) {
        throw new RuntimeException("Name nodes should never be evaluated");
    }

}
