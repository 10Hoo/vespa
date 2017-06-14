// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchlib.rankingexpression.rule;

import com.google.common.collect.ImmutableList;
import com.yahoo.searchlib.rankingexpression.evaluation.BooleanValue;
import com.yahoo.searchlib.rankingexpression.evaluation.Context;
import com.yahoo.searchlib.rankingexpression.evaluation.Value;

import java.util.*;

/**
 * A node which returns true or false depending on a set membership test
 *
 * @author bratseth
 * @since 5.1.21
 */
public class SetMembershipNode extends BooleanNode {

    private final ExpressionNode testValue;

    private final List<ExpressionNode> setValues;

    public SetMembershipNode(ExpressionNode testValue, List<ExpressionNode> setValues) {
        this.testValue = testValue;
        this.setValues = ImmutableList.copyOf(setValues);
    }

    /** The value to check for membership in the set */
    public ExpressionNode getTestValue() { return testValue; }

    /** Returns an immutable list of the values of the set */
    public List<ExpressionNode> getSetValues() { return setValues; }

    @Override
    public List<ExpressionNode> children() {
        ArrayList<ExpressionNode> children = new ArrayList<>();
        children.add(testValue);
        children.addAll(setValues);
        return children;
    }

    @Override
    public String toString(SerializationContext context, Deque<String> path, CompositeNode parent) {
        StringBuilder b = new StringBuilder(testValue.toString(context, path, this));
        b.append(" in [");
        for (int i = 0, len = setValues.size(); i < len; ++i) {
            b.append(setValues.get(i).toString(context, path, this));
            if (i < len - 1) {
                b.append(", ");
            }
        }
        b.append("]");
        return b.toString();
    }

    @Override
    public Value evaluate(Context context) {
        Value value = testValue.evaluate(context);
        for (ExpressionNode setValue : setValues) {
            if (setValue.evaluate(context).equals(value))
                return new BooleanValue(true);
        }
        return new BooleanValue(false);
    }

    @Override
    public SetMembershipNode setChildren(List<ExpressionNode> children) {
        if (children.size()<1) throw new IllegalArgumentException("A set membership test must have at least 1 child");
        return new SetMembershipNode(children.get(0), children.subList(1, children.size()));
    }

}
