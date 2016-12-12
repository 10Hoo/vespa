// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/expression/unaryfunctionnode.h>

namespace search {
    namespace attribute {
        class IAttributeVector;
        class IAttributeContext;
    }
namespace expression {

class ArrayAtLookup : public UnaryFunctionNode
{
public:
    DECLARE_EXPRESSIONNODE(ArrayAtLookup);
    DECLARE_NBO_SERIALIZE;

    ArrayAtLookup();
    ~ArrayAtLookup();

    ArrayAtLookup(const vespalib::string &attribute,
                  const ExpressionNode::CP & arg);

    ArrayAtLookup(const search::attribute::IAttributeVector &attr,
                  const ExpressionNode::CP &indexArg);

    ArrayAtLookup(const ArrayAtLookup &rhs);

    ArrayAtLookup & operator= (const ArrayAtLookup &rhs);

    void setDocId(DocId docId) { _docId = docId; }
private:
    virtual bool onExecute() const;
    virtual void onPrepareResult();
    virtual void wireAttributes(const search::attribute::IAttributeContext &attrCtx);

    enum BasicAttributeType {
        BAT_INT, BAT_FLOAT, BAT_STRING
    };

    vespalib::string _attributeName = vespalib::string();
    const search::attribute::IAttributeVector * _attribute = 0;
    DocId _docId = 0;
    BasicAttributeType _basicAttributeType = BAT_STRING;
};

}
}

