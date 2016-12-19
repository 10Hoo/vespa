// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/common/identifiable.h>
#include <vespa/searchlib/common/hitrank.h>
#include "rawrank.h"


namespace search {
namespace aggregation {

class Hit : public vespalib::Identifiable
{
private:
    RawRank _rank;

public:
    DECLARE_IDENTIFIABLE_ABSTRACT_NS2(search, aggregation, Hit);
    DECLARE_NBO_SERIALIZE;
    typedef std::unique_ptr<Hit> UP;

    Hit() : _rank() {}
    Hit(RawRank rank) : _rank(rank) {}
    RawRank getRank() const { return _rank; }
    virtual Hit *clone() const = 0;
    virtual int onCmp(const Identifiable &b) const;
    virtual void visitMembers(vespalib::ObjectVisitor &visitor) const;
};

}
}

