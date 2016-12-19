// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "hitcollector.h"
#include <vespa/searchlib/fef/feature_resolver.h>
#include <stdexcept>

#include <vespa/log/log.h>
LOG_SETUP(".searchvisitor.hitcollector");

using search::FeatureSet;
using search::fef::MatchData;
using vdslib::SearchResult;

namespace storage {

HitCollector::HitCollector(size_t wantedHits) :
    _hits(),
    _sortedByDocId(true)
{
    _hits.reserve(wantedHits);
}

const vsm::Document &
HitCollector::getDocSum(const search::DocumentIdT & docId) const
{
    for (HitVector::const_iterator it(_hits.begin()), mt(_hits.end()); it < mt; it++) {
        if (docId == it->getDocId()) {
            return *it->getDocument();
        }
    }
    throw std::runtime_error(vespalib::make_string("Could not look up document id %d", docId));
}

bool
HitCollector::addHit(const vsm::StorageDocument::SP & doc, uint32_t docId, const search::fef::MatchData & data, double score)
{
    Hit h(doc, docId, data, score);
    return addHit(h);
}

bool
HitCollector::addHit(const vsm::StorageDocument::SP & doc, uint32_t docId, const search::fef::MatchData & data,
                     double score, const void * sortData, size_t sortDataLen)
{
    Hit h(doc, docId, data, score, sortData, sortDataLen);
    return addHit(h);
}

void
HitCollector::sortByDocId()
{
    if (!_sortedByDocId) {
        std::sort(_hits.begin(), _hits.end()); // sort on docId
        _sortedByDocId = true;
    }
}

bool
HitCollector::addHitToHeap(const Hit & hit) const
{
    // return true if the given hit is better than the current worst one.
    return (hit.getSortBlob().empty())
        ? (hit.cmpRank(_hits[0]) < 0)
        : (hit.cmpSort(_hits[0]) < 0);
}

bool
HitCollector::addHit(const Hit & hit)
{
    bool amongTheBest(false);
    ssize_t avail = (_hits.capacity() - _hits.size());
    bool useSortBlob( ! hit.getSortBlob().empty() );
    if (avail > 1) {
        // No heap yet.
        _hits.push_back(hit);
        amongTheBest = true;
    } else if (_hits.capacity() == 0) {
        // this happens when wantedHitCount = 0
        // in this case we shall not put anything on the heap (which is empty)
    } else if ( avail == 0 && addHitToHeap(hit)) { // already a heap
        if (useSortBlob) {
            std::pop_heap(_hits.begin(), _hits.end(), Hit::SortComparator());
        } else {
            std::pop_heap(_hits.begin(), _hits.end(), Hit::RankComparator());
        }

        _hits.back() = hit;
        amongTheBest = true;

        if (useSortBlob) {
            std::push_heap(_hits.begin(), _hits.end(), Hit::SortComparator());
        } else {
            std::push_heap(_hits.begin(), _hits.end(), Hit::RankComparator());
        }
    } else if (avail == 1) { // make a heap of the hit vector
        _hits.push_back(hit);
        amongTheBest = true;
        if (useSortBlob) {
            std::make_heap(_hits.begin(), _hits.end(), Hit::SortComparator());
        } else {
            std::make_heap(_hits.begin(), _hits.end(), Hit::RankComparator());
        }
        _sortedByDocId = false; // the hit vector is no longer sorted by docId
    }
    return amongTheBest;
}

void
HitCollector::fillSearchResult(vdslib::SearchResult & searchResult)
{
    sortByDocId();
    for (HitVector::const_iterator it(_hits.begin()), mt(_hits.end()); it != mt; it++) {
        vespalib::string documentId(it->getDocument()->docDoc().getId().toString());
        search::DocumentIdT docId = it->getDocId();
        SearchResult::RankType rank = it->getRankScore();

        LOG(debug, "fillSearchResult: gDocId(%s), lDocId(%u), rank(%f)", documentId.c_str(), docId, (float)rank);

        if (it->getSortBlob().empty()) {
            searchResult.addHit(docId, documentId.c_str(), rank);
        } else {
            searchResult.addHit(docId, documentId.c_str(), rank, it->getSortBlob().c_str(), it->getSortBlob().size());
        }
    }
}

FeatureSet::SP
HitCollector::getFeatureSet(IRankProgram &rankProgram,
                            const search::fef::FeatureResolver &resolver)
{
    if (resolver.num_features() == 0 || _hits.empty()) {
        return FeatureSet::SP(new FeatureSet());
    }
    sortByDocId();
    std::vector<vespalib::string> names;
    names.reserve(resolver.num_features());
    for (size_t i = 0; i < resolver.num_features(); ++i) {
        names.emplace_back(resolver.name_of(i));
    }
    FeatureSet::SP retval = FeatureSet::SP(new FeatureSet(names, _hits.size()));
    for (HitVector::iterator it(_hits.begin()), mt(_hits.end()); it != mt; ++it) {
        rankProgram.run(it->getDocId(), it->getMatchData());
        uint32_t docId = it->getDocId();
        search::feature_t * f = retval->getFeaturesByIndex(retval->addDocId(docId));
        for (uint32_t j = 0; j < names.size(); ++j) {
            f[j] = *resolver.resolve_number(j);
            LOG(debug, "getFeatureSet: lDocId(%u), '%s': %f", docId, names[j].c_str(), f[j]);
        }
    }
    return retval;
}

} // namespace storage

