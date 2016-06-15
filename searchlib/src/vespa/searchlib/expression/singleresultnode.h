// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/expression/resultnode.h>

namespace search {
namespace expression {

class SingleResultNode : public ResultNode
{
public:
    virtual ~SingleResultNode() { }
    DECLARE_ABSTRACT_RESULTNODE(SingleResultNode);
    typedef vespalib::IdentifiablePtr<SingleResultNode> CP;
    typedef std::unique_ptr<SingleResultNode> UP;
    virtual SingleResultNode *clone() const = 0;

    virtual void min(const ResultNode & b) = 0;
    virtual void max(const ResultNode & b) = 0;
    virtual void add(const ResultNode & b) = 0;

    virtual void setMin() = 0;
    virtual void setMax() = 0;
    virtual size_t getRawByteSize() const { return onGetRawByteSize(); }
    virtual size_t onGetRawByteSize() const = 0;
};

}
}

