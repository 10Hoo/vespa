// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "searchiterator.h"
#include "fake_result.h"
#include <vespa/searchlib/fef/termfieldmatchdataarray.h>

namespace search {
namespace queryeval {

class FakeSearch : public SearchIterator
{
private:
    vespalib::string             _tag;
    vespalib::string             _field;
    vespalib::string             _term;
    FakeResult                   _result;
    uint32_t                     _offset;
    fef::TermFieldMatchDataArray _tfmda;

    bool valid() const { return _offset < _result.inspect().size(); }
    uint32_t currId() const { return _result.inspect()[_offset].docId; }
    void next() { ++_offset; }

public:
    FakeSearch(const vespalib::string &tag,
               const vespalib::string &field,
               const vespalib::string &term,
               const FakeResult &res,
               const fef::TermFieldMatchDataArray &tfmda)
        : _tag(tag), _field(field), _term(term),
          _result(res), _offset(0), _tfmda(tfmda)
    {
        assert(_tfmda.size() == 1);
    }
    virtual void doSeek(uint32_t docid);
    virtual void doUnpack(uint32_t docid);
    virtual const PostingInfo *getPostingInfo() const { return _result.postingInfo(); }
    virtual void visitMembers(vespalib::ObjectVisitor &visitor) const;
};

} // namespace queryeval
} // namespace search

