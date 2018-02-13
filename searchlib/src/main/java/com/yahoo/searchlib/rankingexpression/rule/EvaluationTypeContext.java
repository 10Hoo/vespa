// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchlib.rankingexpression.rule;

import com.yahoo.tensor.TensorType;
import com.yahoo.tensor.evaluation.TypeContext;

public interface EvaluationTypeContext extends TypeContext {

    TensorType getType(String name, Arguments arguments, String output);

}
