// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/expression/singleresultnode.h>

namespace search {
namespace expression {

class NumericResultNode : public SingleResultNode
{
public:
    DECLARE_ABSTRACT_EXPRESSIONNODE(NumericResultNode);
    typedef vespalib::IdentifiablePtr<NumericResultNode> CP;
    typedef std::unique_ptr<NumericResultNode> UP;
    virtual NumericResultNode *clone() const = 0;
    virtual void multiply(const ResultNode & b) = 0;
    virtual void divide(const ResultNode & b) = 0;
    virtual void modulo(const ResultNode & b) = 0;
};

}
}

