// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".queryeval.weighted_set_term.blueprint");

#include "weighted_set_term_blueprint.h"
#include "weighted_set_term_search.h"
#include <vespa/searchlib/fef/termfieldmatchdata.h>
#include <vespa/searchlib/queryeval/searchiterator.h>
#include <vespa/vespalib/objects/visit.h>
#include <algorithm>

namespace search {
namespace queryeval {

WeightedSetTermBlueprint::WeightedSetTermBlueprint(const FieldSpec &field)
    : ComplexLeafBlueprint(field),
      _estimate(),
      _weights(),
      _terms()
{
}

WeightedSetTermBlueprint::~WeightedSetTermBlueprint()
{
    while (!_terms.empty()) {
        delete _terms.back();
        _terms.pop_back();
    }
}

void
WeightedSetTermBlueprint::addTerm(Blueprint::UP term, int32_t weight)
{
    HitEstimate childEst = term->getState().estimate();
    if (! childEst.empty) {
        if (_estimate.empty) {
            _estimate = childEst;
        } else {
            _estimate.estHits += childEst.estHits;
        }
        setEstimate(_estimate);
    }
    _weights.push_back(weight);
    _terms.push_back(term.get());
    term.release();
}

SearchIterator::UP
WeightedSetTermBlueprint::createSearch(search::fef::MatchData &md,
                                       bool) const
{
    const State &state = getState();
    assert(state.numFields() == 1);
    search::fef::TermFieldMatchData &tfmd = *state.field(0).resolve(md);

    std::vector<SearchIterator*> children(_terms.size());
    for (size_t i = 0; i < _terms.size(); ++i) {
        children[i] = _terms[i]->createSearch(md, true).release();
    }
    return SearchIterator::UP(WeightedSetTermSearch::create(children, tfmd, _weights));
}

void
WeightedSetTermBlueprint::fetchPostings(bool strict)
{
    (void) strict;
    for (size_t i = 0; i < _terms.size(); ++i) {
        _terms[i]->fetchPostings(true);
    }
}

void
WeightedSetTermBlueprint::visitMembers(vespalib::ObjectVisitor &visitor) const
{
    LeafBlueprint::visitMembers(visitor);
    visit(visitor, "_weights", _weights);
    visit(visitor, "_terms", _terms);
}

SearchIterator::UP
WeightedSetTermBlueprint::createLeafSearch(const search::fef::TermFieldMatchDataArray &, bool) const
{
    abort();
}

}  // namespace search::queryeval
}  // namespace search
