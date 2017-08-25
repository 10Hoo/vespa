// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/common/idocumentmetastore.h>
#include <vespa/searchlib/aggregation/grouping.h>
#include <vespa/searchcore/grouping/groupingcontext.h>

namespace search {

namespace grouping {

/**
 * Wrapper class used to handle actual grouping. All input data is
 * assumed to be kept alive by the user.
 **/
class GroupingManager
{
private:
    GroupingManager(const GroupingManager &);
    GroupingManager &operator=(const GroupingManager &);

    GroupingContext         &_groupingContext;

public:
    /**
     * Create a new grouping manager.
     *
     * @param groupingContext Context to use for grouping
     **/
    GroupingManager(GroupingContext & groupingContext);

    /**
     * Release resources
     **/
    ~GroupingManager();

    /**
     * @return true if this manager is holding an empty grouping request.
     **/
    bool empty() const { return _groupingContext.getGroupingList().empty(); }

    /**
     * Initialize underlying context with attribute bindings.
     *
     * @param attrCtx attribute context
     **/
    void init(const search::attribute::IAttributeContext &attrCtx);

    /**
     * Perform actual grouping on the given results.
     * The results must be in relevance sort order.
     * Will only perform grouping that will not resort.
     *
     * @param searchResults the result set in array form
     * @param binSize size of search result array
     **/
    void groupInRelevanceOrder(const RankedHit *searchResults, uint32_t binSize);

    /**
     * Perform actual grouping on the given the results.
     * The results should be in fastest access order which is normally unsorted.
     * Will only perform grouping that actually will resort.
     *
     * @param searchResults the result set in array form
     * @param binSize size of search result array
     * @param overflow The unranked hits.
     **/
    void groupUnordered(const RankedHit *searchResults, uint32_t binSize, const search::BitVector * overflow);

    /**
     * Merge another grouping context into the underlying context of
     * this manager. Both contexts must have the same groupings in the
     * same order.
     *
     * @param ctx context to merge into the underlying context of this manager
     **/
    void merge(GroupingContext &ctx);

    /**
     * Called after merge has been called (possibly multiple times) to
     * prune unwanted information from the underlying grouping
     * context.
     **/
    void prune();

    /**
     * Perform converting from local to global document id on all hits
     * in the underlying grouping trees.
     *
     * @param metaStore the attribute used to map from lid to gid.
     **/
    void convertToGlobalId(const search::IDocumentMetaStore &metaStore);
};

} // namespace search::grouping
} // namespace search

