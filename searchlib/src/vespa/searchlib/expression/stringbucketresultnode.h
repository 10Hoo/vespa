// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "bucketresultnode.h"
#include "stringresultnode.h"

namespace search {
namespace expression {

class StringBucketResultNode : public BucketResultNode
{
private:
    ResultNode::CP _from;
    ResultNode::CP _to;
    static StringBucketResultNode _nullResult;
    virtual size_t onGetRawByteSize() const { return sizeof(_from) + sizeof(_to); }
public:
    struct GetValue {
        BufferRef _tmp;
        ConstBufferRef operator () (const ResultNode & r) { return r.getString(_tmp); }
    };

    DECLARE_EXPRESSIONNODE(StringBucketResultNode);
    DECLARE_NBO_SERIALIZE;
    StringBucketResultNode() : _from(new StringResultNode()), _to(new StringResultNode()) {}
    StringBucketResultNode(const vespalib::stringref & from, const vespalib::stringref & to) : _from(new StringResultNode(from)), _to(new StringResultNode(to)) {}
    StringBucketResultNode(ResultNode::UP from, ResultNode::UP to) : _from(from.release()), _to(to.release()) {}
    virtual size_t hash() const;
    virtual int onCmp(const Identifiable & b) const;
    int contains(const StringBucketResultNode & b) const;
    int contains(const ConstBufferRef & v) const { return contains(v.c_str()); }
    int contains(const char * v) const;
    virtual void visitMembers(vespalib::ObjectVisitor &visitor) const;
    StringBucketResultNode &setRange(const vespalib::stringref & from, const vespalib::stringref & to) {
        _from.reset(new StringResultNode(from));
        _to.reset(new StringResultNode(to));
        return *this;
    }
    virtual const StringBucketResultNode& getNullBucket() const override { return getNull(); }
    static const StringBucketResultNode & getNull() { return _nullResult; }
};

}
}

