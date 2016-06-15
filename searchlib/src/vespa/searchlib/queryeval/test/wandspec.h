// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "leafspec.h"
#include "trackedsearch.h"
#include <vespa/searchlib/fef/matchdatalayout.h>
#include <vespa/searchlib/queryeval/fake_search.h>
#include <vespa/searchlib/queryeval/searchiterator.h>
#include <vespa/searchlib/queryeval/wand/wand_parts.h>
#include <vector>

namespace search {
namespace queryeval {
namespace test {

/**
 * Defines the overall behavior of a wand like search with tracked children.
 * This struct also owns the search iterator history.
 **/
class WandSpec
{
private:
    std::vector<LeafSpec>             _leafs;
    fef::MatchDataLayout              _layout;
    std::vector<fef::TermFieldHandle> _handles;
    SearchHistory                     _history;

public:
    WandSpec() : _leafs(), _layout(), _handles(), _history() {}
    WandSpec &leaf(const LeafSpec &l) {
        _leafs.push_back(l);
        _handles.push_back(_layout.allocTermField(0));
        return *this;
    }
    wand::Terms getTerms(fef::MatchData *matchData = NULL) {
        wand::Terms terms;
        for (size_t i = 0; i < _leafs.size(); ++i) {
            fef::TermFieldMatchData *tfmd = (matchData != NULL ? matchData->resolveTermField(_handles[i]) : NULL);
            terms.push_back(wand::Term(_leafs[i].create(_history, tfmd),
                                       _leafs[i].weight,
                                       _leafs[i].result.inspect().size(),
                                       tfmd));
        }
        return terms;
    }
    SearchHistory &getHistory() { return _history; }
    fef::MatchData::UP createMatchData() const { return _layout.createMatchData(); }
};

} // namespace test
} // namespace queryeval
} // namespace search

