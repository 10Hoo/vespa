package com.yahoo.tensor.functions;

import com.yahoo.tensor.Tensor;

/**
 * A composite tensor function is a tensor function which can be expressed (less tersely)
 * as a tree of primitive tensor functions.
 * 
 * @author bratseth
 */
public abstract class CompositeTensorFunction extends TensorFunction {

    /** Evaluates this by first converting it to a primitive function */
    @Override
    public final Tensor evaluate(EvaluationContext context) { return toPrimitive().evaluate(context); }

}
