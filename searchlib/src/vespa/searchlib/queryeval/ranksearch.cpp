// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "ranksearch.h"

namespace search {
namespace queryeval {

void
RankSearch::doSeek(uint32_t docid)
{
    SearchIterator & firstChild(**getChildren().begin());
    if (firstChild.seek(docid)) {
        setDocId(docid);
    }
}

namespace {
/**
 * A simple implementation of the strict Rank search operation.
 **/
class RankSearchStrict : public RankSearch
{
protected:
    void doSeek(uint32_t docid) override;
    UP andWith(UP filter, uint32_t estimate) override;;

public:
    /**
     * Create a new Rank Search with the given children and
     * strictness. A strict Rank can assume that the first child below
     * is also strict. No such assumptions can be made about the other
     * children.
     *
     * @param children the search objects we are rank'ing
     **/
    RankSearchStrict(const Children & children) : RankSearch(children) { }
};

SearchIterator::UP
RankSearchStrict::andWith(UP filter, uint32_t estimate)
{
    return getChildren()[0]->andWith(std::move(filter), estimate);
}

void
RankSearchStrict::doSeek(uint32_t docid)
{
    SearchIterator & firstChild(**getChildren().begin());
    setDocId(firstChild.seek(docid) ? docid : firstChild.getDocId());
}
}  // namespace

SearchIterator *
RankSearch::create(const RankSearch::Children &children, bool strict) {
    if (strict) {
        return new RankSearchStrict(children);
    } else {
        return new RankSearch(children);
    }
}

}  // namespace queryeval
}  // namespace search
