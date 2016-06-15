// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/query/base.h>
#include <vespa/vespalib/objects/nbostream.h>
#include <vespa/vespalib/util/linkedptr.h>
#include <vector>

namespace proton {

class LidVectorContext
{
public:
    typedef std::vector<search::DocumentIdT> LidVector;
private:
    LidVector _result;
    size_t    _docIdLimit;
    enum { ARRAY = 0, BITVECTOR = 1 };
public:
    typedef vespalib::LinkedPtr<LidVectorContext> LP;
    LidVectorContext();
    LidVectorContext(size_t docIdLimit);
    LidVectorContext(size_t docIdLimit, const LidVector &lids);
    void addLid(const search::DocumentIdT lid);
    void serialize(vespalib::nbostream &os) const;
    void deserialize(vespalib::nbostream &is);
    const LidVector &getLidVector() const { return _result; }
    void clearLidVector() { _result.clear(); }
    size_t getDocIdLimit() const { return _docIdLimit; }
    size_t getNumLids() const { return _result.size(); }
};

} // namespace proton

