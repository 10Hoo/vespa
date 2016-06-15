// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once


namespace search
{

namespace queryeval
{

class SearchIterator;
}

namespace fef
{

class TermFieldMatchData;
}


namespace attribute
{


/**
 * Interface for search context helper classes to create attribute
 * search iterators based on posting lists and using dictionary
 * information to better estimate number of hits.  Also used for
 * enumerated attributes without posting lists to eliminate brute
 * force searches for nonexisting values.
 */

class IPostingListSearchContext
{
protected:

    IPostingListSearchContext(void)
    {
    }

    virtual
    ~IPostingListSearchContext(void)
    {
    }

public:
    virtual void
    fetchPostings(bool strict) = 0;

    virtual std::unique_ptr<queryeval::SearchIterator>
    createPostingIterator(fef::TermFieldMatchData *matchData, bool strict) = 0;

    virtual unsigned int
    approximateHits(void) const = 0;
};


} // namespace attribute

} // namespace search

