// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".searchlib.attribute.attribute_blueprint_factory");

#include "attribute_blueprint_factory.h"
#include "attribute_weighted_set_blueprint.h"
#include "i_document_weight_attribute.h"
#include "iterator_pack.h"
#include "predicate_attribute.h"
#include <vespa/searchlib/attribute/attributeguard.h>
#include <vespa/searchlib/attribute/attributevector.h>
#include <vespa/searchlib/attribute/iattributemanager.h>
#include <vespa/searchlib/common/location.h>
#include <vespa/searchlib/common/locationiterators.h>
#include <vespa/searchlib/fef/termfieldmatchdata.h>
#include <vespa/searchlib/query/tree/stackdumpcreator.h>
#include <vespa/searchlib/queryeval/andsearchstrict.h>
#include <vespa/searchlib/queryeval/create_blueprint_visitor_helper.h>
#include <vespa/searchlib/queryeval/document_weight_search_iterator.h>
#include <vespa/searchlib/queryeval/dot_product_blueprint.h>
#include <vespa/searchlib/queryeval/dot_product_search.h>
#include <vespa/searchlib/queryeval/emptysearch.h>
#include <vespa/searchlib/queryeval/get_weight_from_node.h>
#include <vespa/searchlib/queryeval/intermediate_blueprints.h>
#include <vespa/searchlib/queryeval/leaf_blueprints.h>
#include <vespa/searchlib/queryeval/orlikesearch.h>
#include <vespa/searchlib/queryeval/orsearch.h>
#include <vespa/searchlib/queryeval/predicate_blueprint.h>
#include <vespa/searchlib/queryeval/searchiterator.h>
#include <vespa/searchlib/queryeval/termasstring.h>
#include <vespa/searchlib/queryeval/wand/parallel_weak_and_blueprint.h>
#include <vespa/searchlib/queryeval/wand/parallel_weak_and_search.h>
#include <vespa/searchlib/queryeval/wand/weak_and_heap.h>
#include <vespa/searchlib/queryeval/weighted_set_term_search.h>
#include <sstream>
#include <utility>
#include <vespa/vespalib/util/regexp.h>

using search::AttributeVector;
using search::fef::TermFieldMatchData;
using search::fef::TermFieldMatchDataArray;
using search::fef::TermFieldMatchDataPosition;
using search::query::Location;
using search::query::LocationTerm;
using search::query::Node;
using search::query::NumberTerm;
using search::query::PredicateQuery;
using search::query::PrefixTerm;
using search::query::RangeTerm;
using search::query::StackDumpCreator;
using search::query::StringTerm;
using search::query::SubstringTerm;
using search::query::SuffixTerm;
using search::query::RegExpTerm;
using search::queryeval::AndBlueprint;
using search::queryeval::AndSearchStrict;
using search::queryeval::Blueprint;
using search::queryeval::CreateBlueprintVisitorHelper;
using search::queryeval::DotProductBlueprint;
using search::queryeval::FieldSpec;
using search::queryeval::FieldSpecBaseList;
using search::queryeval::MultiSearch;
using search::queryeval::OrLikeSearch;
using search::queryeval::OrSearch;
using search::queryeval::ParallelWeakAndBlueprint;
using search::queryeval::PredicateBlueprint;
using search::queryeval::SearchIterator;
using search::queryeval::Searchable;
using search::queryeval::NoUnpack;
using search::queryeval::IRequestContext;
using search::queryeval::WeightedSetTermBlueprint;
using vespalib::geo::ZCurve;
using vespalib::string;

namespace search {
namespace {

//-----------------------------------------------------------------------------

/**
 * Blueprint for creating regular, stack-based attribute iterators.
 **/
class AttributeFieldBlueprint :
        public search::queryeval::SimpleLeafBlueprint
{
private:
    AttributeVector::SearchContext::UP _search_context;

    AttributeFieldBlueprint(const FieldSpec &field,
                            const AttributeVector &attribute,
                            const string &query_stack,
                            const AttributeVector::SearchContext::Params &params)
        : SimpleLeafBlueprint(field),
          _search_context(attribute.getSearch(query_stack, params).release())
    {
        uint32_t estHits = _search_context->approximateHits();
        HitEstimate estimate(estHits, estHits == 0);
        setEstimate(estimate);
    }

public:
    AttributeFieldBlueprint(const FieldSpec &field,
                            const AttributeVector &attribute,
                            const string &query_stack)
        : AttributeFieldBlueprint(field,
                                  attribute,
                                  query_stack,
                                  AttributeVector::SearchContext::Params()
                                  .useBitVector(field.isFilter()))
    {
    }

    AttributeFieldBlueprint(const FieldSpec &field,
                            const AttributeVector &attribute,
                            const AttributeVector &diversity,
                            const string &query_stack,
                            size_t diversityCutoffGroups,
                            bool diversityCutoffStrict)
        : AttributeFieldBlueprint(field,
                                  attribute,
                                  query_stack,
                                  AttributeVector::SearchContext::Params()
                                      .diversityAttribute(&diversity)
                                      .useBitVector(field.isFilter())
                                      .diversityCutoffGroups(diversityCutoffGroups)
                                      .diversityCutoffStrict(diversityCutoffStrict))
    {
    }

    virtual SearchIterator::UP
    createLeafSearch(const TermFieldMatchDataArray &tfmda, bool strict) const
    {
        assert(tfmda.size() == 1);
        return _search_context->createIterator(tfmda[0], strict);
    }

    virtual void
    fetchPostings(bool strict)
    {
        _search_context->fetchPostings(strict);
    }

    virtual void visitMembers(vespalib::ObjectVisitor &visitor) const;
};

void
AttributeFieldBlueprint::visitMembers(vespalib::ObjectVisitor &visitor) const
{
    search::queryeval::LeafBlueprint::visitMembers(visitor);
    visit(visitor, "attribute", _search_context->attribute().getName());
}

//-----------------------------------------------------------------------------

template <bool is_strict>
struct LocationPreFilterIterator : public OrLikeSearch<is_strict, NoUnpack>
{
    LocationPreFilterIterator(const MultiSearch::Children &children) : OrLikeSearch<is_strict, NoUnpack>(children, NoUnpack()) {}
    virtual void doUnpack(uint32_t) override {}
};

class LocationPreFilterBlueprint :
        public search::queryeval::ComplexLeafBlueprint
{
private:
    const AttributeVector & _attribute;
    std::vector<AttributeVector::SearchContext::UP> _rangeSearches;
    bool _should_use;

public:
    LocationPreFilterBlueprint(const FieldSpec &field, const AttributeVector &attribute, const ZCurve::RangeVector &rangeVector)
        : ComplexLeafBlueprint(field),
          _attribute(attribute),
          _rangeSearches(),
          _should_use(false)
    {
        uint64_t estHits(0);
        const AttributeVector & attr(_attribute);
        for (auto it(rangeVector.begin()), mt(rangeVector.end()); it != mt; it++) {
            const ZCurve::Range &r(*it);
            search::query::Range qr(r.min(), r.max());
            search::query::SimpleRangeTerm rt(qr, "", 0, search::query::Weight(0));
            string stack(StackDumpCreator::create(rt));
            _rangeSearches.push_back(attr.getSearch(stack, AttributeVector::SearchContext::Params()));
            estHits += _rangeSearches.back()->approximateHits();
            LOG(debug, "Range '%s' estHits %ld", qr.getRangeString().c_str(), estHits);
        }
        if (estHits > attr.getNumDocs()) {
            estHits = attr.getNumDocs();
        }
        if (estHits * 10 < attr.getNumDocs()) {
            _should_use = true;
        }
        HitEstimate estimate(estHits, estHits == 0);
        setEstimate(estimate);        
    }

    bool should_use() const { return _should_use; }

    virtual SearchIterator::UP
    createLeafSearch(const TermFieldMatchDataArray &tfmda, bool strict) const override
    {
        MultiSearch::Children children;
        for (auto it(_rangeSearches.begin()), mt(_rangeSearches.end()); it != mt; it++) {
            children.push_back((*it)->createIterator(tfmda[0],
                                                     strict).release());
        }
        if (strict) {
            return SearchIterator::UP(new LocationPreFilterIterator<true>(children));
        } else {
            return SearchIterator::UP(new LocationPreFilterIterator<false>(children));
        }
    }

    virtual void fetchPostings(bool strict) override {
        for (size_t i(0); i < _rangeSearches.size(); i++) {
            _rangeSearches[i]->fetchPostings(strict);
        }
    }
};

//-----------------------------------------------------------------------------

class LocationPostFilterBlueprint :
        public search::queryeval::ComplexLeafBlueprint
{
private:
    const AttributeVector & _attribute;
    search::common::Location _location;

public:
    LocationPostFilterBlueprint(const FieldSpec &field, const AttributeVector &attribute, const Location &loc)
        : ComplexLeafBlueprint(field),
          _attribute(attribute),
          _location()
    {
        _location.setVec(attribute);
        _location.parse(loc.getLocationString());
        uint32_t estHits = _attribute.getNumDocs();
        HitEstimate estimate(estHits, estHits == 0);
        setEstimate(estimate);
    }

    const search::common::Location &location() const { return _location; }

    virtual SearchIterator::UP
    createLeafSearch(const TermFieldMatchDataArray &, bool strict) const override
    {
        unsigned int num_docs = _attribute.getNumDocs();
        return SearchIterator::UP(FastS_AllocLocationIterator(num_docs, strict, _location));
    }
};

//-----------------------------------------------------------------------------

Blueprint::UP make_location_blueprint(const FieldSpec &field, const AttributeVector &attribute, const Location &loc) {
    LocationPostFilterBlueprint *post_filter = new LocationPostFilterBlueprint(field, attribute, loc);
    Blueprint::UP post_filter_bp(post_filter);
    const search::common::Location &location = post_filter->location();
    if (location.getMinX() > location.getMaxX() ||
        location.getMinY() > location.getMaxY())
    {
        return Blueprint::UP(new queryeval::EmptyBlueprint(field));
    }
    ZCurve::RangeVector rangeVector = ZCurve::find_ranges(
            location.getMinX(), location.getMinY(),
            location.getMaxX(), location.getMaxY());
    LocationPreFilterBlueprint *pre_filter = new LocationPreFilterBlueprint(field, attribute, rangeVector);
    Blueprint::UP pre_filter_bp(pre_filter);
    if (!pre_filter->should_use()) {
        return post_filter_bp;
    }
    AndBlueprint *root = new AndBlueprint();
    Blueprint::UP root_bp(root);
    root->addChild(std::move(pre_filter_bp));
    root->addChild(std::move(post_filter_bp));
    return root_bp;
}

//-----------------------------------------------------------------------------

template <typename SearchType>
class DirectWeightedSetBlueprint : public search::queryeval::ComplexLeafBlueprint
{
private:
    HitEstimate                                         _estimate;
    std::vector<int32_t>                                _weights;
    std::vector<IDocumentWeightAttribute::LookupResult> _terms;
    const IDocumentWeightAttribute                     &_attr;

public:
    DirectWeightedSetBlueprint(const FieldSpec &field,
                              const IDocumentWeightAttribute &attr, size_t size_hint)
        : ComplexLeafBlueprint(field),
          _estimate(),
          _weights(),
          _terms(),
          _attr(attr)
    {
        _weights.reserve(size_hint);
        _terms.reserve(size_hint);
    }

    void addTerm(const vespalib::string &term, int32_t weight) {
        IDocumentWeightAttribute::LookupResult result = _attr.lookup(term);
        HitEstimate childEst(result.posting_size, (result.posting_size == 0));
        if (!childEst.empty) {
            if (_estimate.empty) {
                _estimate = childEst;
            } else {
                _estimate.estHits += childEst.estHits;
            }
            setEstimate(_estimate);
            _weights.push_back(weight);
            _terms.push_back(result);
        }
    }

    SearchIterator::UP createLeafSearch(const TermFieldMatchDataArray &tfmda, bool) const
    {
        assert(tfmda.size() == 1);
        if (_terms.size() == 0) {
            return SearchIterator::UP(new search::queryeval::EmptySearch());
        }
        std::vector<DocumentWeightIterator> iterators;
        const size_t numChildren = _terms.size();
        iterators.reserve(numChildren);
        for (const IDocumentWeightAttribute::LookupResult &r : _terms) {
            _attr.create(r.posting_idx, iterators);
        }
        return SearchType::create(*tfmda[0], _weights, std::move(iterators));
    }
};

//-----------------------------------------------------------------------------

class DirectWandBlueprint : public search::queryeval::ComplexLeafBlueprint
{
private:
    HitEstimate                                         _estimate;
    mutable queryeval::SharedWeakAndPriorityQueue       _scores;
    const queryeval::wand::score_t                      _scoreThreshold;
    double                                              _thresholdBoostFactor;
    const uint32_t                                      _scoresAdjustFrequency;
    std::vector<int32_t>                                _weights;
    std::vector<IDocumentWeightAttribute::LookupResult> _terms;
    const IDocumentWeightAttribute                     &_attr;

public:
    DirectWandBlueprint(const FieldSpec &field,
                        const IDocumentWeightAttribute &attr,
                        uint32_t scoresToTrack,
                        queryeval::wand::score_t scoreThreshold,
                        double thresholdBoostFactor,
                        size_t size_hint)
        : ComplexLeafBlueprint(field),
          _estimate(),
          _scores(scoresToTrack),
          _scoreThreshold(scoreThreshold),
          _thresholdBoostFactor(thresholdBoostFactor),
          _scoresAdjustFrequency(queryeval::DEFAULT_PARALLEL_WAND_SCORES_ADJUST_FREQUENCY),
          _weights(),
          _terms(),
          _attr(attr)
    {
        _weights.reserve(size_hint);
        _terms.reserve(size_hint);
    }

    void addTerm(const vespalib::string &term, int32_t weight) {
        IDocumentWeightAttribute::LookupResult result = _attr.lookup(term);
        HitEstimate childEst(result.posting_size, (result.posting_size == 0));
        if (!childEst.empty) {
            if (_estimate.empty) {
                _estimate = childEst;
            } else {
                _estimate.estHits += childEst.estHits;
            }
            setEstimate(_estimate);
            _weights.push_back(weight);
            _terms.push_back(result);
        }
    }

    SearchIterator::UP createLeafSearch(const TermFieldMatchDataArray &tfmda, bool strict) const
    {
        assert(tfmda.size() == 1);
        if (_terms.size() == 0) {
            return SearchIterator::UP(new search::queryeval::EmptySearch());
        }
        return search::queryeval::ParallelWeakAndSearch::create(*tfmda[0],
                queryeval::ParallelWeakAndSearch::MatchParams(_scores,
                        _scoreThreshold,
                        _thresholdBoostFactor,
                        _scoresAdjustFrequency).setDocIdLimit(get_docid_limit()),
                _weights, _terms, _attr, strict);
    }
};

//-----------------------------------------------------------------------------

class DirectAttributeBlueprint : public search::queryeval::SimpleLeafBlueprint
{
private:
    vespalib::string                        _attrName;
    const IDocumentWeightAttribute         &_attr;
    IDocumentWeightAttribute::LookupResult  _dict_entry;

public:
    DirectAttributeBlueprint(const FieldSpec &field,
                             const vespalib::string & name,
                             const IDocumentWeightAttribute &attr, const vespalib::string &term)
        : SimpleLeafBlueprint(field),
          _attrName(name),
          _attr(attr),
          _dict_entry(_attr.lookup(term))
    {
        setEstimate(HitEstimate(_dict_entry.posting_size, (_dict_entry.posting_size == 0)));
    }

    SearchIterator::UP createLeafSearch(const TermFieldMatchDataArray &tfmda, bool) const
    {
        assert(tfmda.size() == 1);
        if (_dict_entry.posting_size == 0) {
            return SearchIterator::UP(new search::queryeval::EmptySearch());
        }
        return SearchIterator::UP(new queryeval::DocumentWeightSearchIterator(*tfmda[0], _attr, _dict_entry));
    }

    virtual void visitMembers(vespalib::ObjectVisitor &visitor) const
    {   
        search::queryeval::LeafBlueprint::visitMembers(visitor);
        visit(visitor, "attribute", _attrName);
    }
};

//-----------------------------------------------------------------------------

bool check_valid_diversity_attr(const AttributeVector *attr) {
    if (attr == nullptr) {
        return false;
    }
    if (attr->hasMultiValue()) {
        return false;
    }
    return (attr->hasEnum() || attr->isIntegerType() || attr->isFloatingPointType());
}

//-----------------------------------------------------------------------------


/**
 * Determines the correct Blueprint to use.
 **/
class CreateBlueprintVisitor : public CreateBlueprintVisitorHelper
{
private:
    const FieldSpec &_field;
    const AttributeVector & _attr;
    const IDocumentWeightAttribute *_dwa;

public:
    CreateBlueprintVisitor(Searchable &searchable,
                           const IRequestContext &requestContext,
                           const FieldSpec &field,
                           const AttributeVector &attr)
        : CreateBlueprintVisitorHelper(searchable, field, requestContext),
          _field(field),
          _attr(attr),
          _dwa(attr.asDocumentWeightAttribute()) {}

    template <class TermNode>
    void visitTerm(TermNode &n, bool simple = false) {
        if (simple && (_dwa != nullptr) && !_field.isFilter() && n.isRanked()) {
            vespalib::string term = search::queryeval::termAsString(n);
            setResult(make_UP(new DirectAttributeBlueprint(_field, _attr.getName(), *_dwa, term)));
        } else {
            const string stack = StackDumpCreator::create(n);
            setResult(make_UP(new AttributeFieldBlueprint(_field, _attr, stack)));
        }
    }

    void visitLocation(LocationTerm &node) {
        Location loc(node.getTerm());
        setResult(make_location_blueprint(_field, _attr, loc));
    }

    void visitPredicate(PredicateQuery &query) {
        const PredicateAttribute *attr =
            dynamic_cast<const PredicateAttribute *>(&_attr);
        if (!attr) {
            LOG(warning, "Trying to apply a PredicateQuery node to a "
                "non-predicate attribute.");
            setResult(Blueprint::UP(new queryeval::EmptyBlueprint(_field)));
        } else {
            setResult(Blueprint::UP(new PredicateBlueprint( _field, *attr, query)));
        }
    }

    virtual void visit(NumberTerm & n) { visitTerm(n, true); }
    virtual void visit(LocationTerm &n) { visitLocation(n); }
    virtual void visit(PrefixTerm & n) { visitTerm(n); }

    virtual void visit(RangeTerm &n) {
        const string stack = StackDumpCreator::create(n);
        const string term = search::queryeval::termAsString(n);
        search::QueryTermSimple parsed_term(term, search::QueryTermSimple::WORD);
        if (parsed_term.getMaxPerGroup() > 0) {
            const AttributeVector * diversity(getRequestContext().getAttribute(parsed_term.getDiversityAttribute()));
            if (check_valid_diversity_attr(diversity)) {
                setResult(make_UP(new AttributeFieldBlueprint(_field, _attr, *diversity, stack,
                                                              parsed_term.getDiversityCutoffGroups(),
                                                              parsed_term.getDiversityCutoffStrict())));
            } else {
                setResult(Blueprint::UP(new queryeval::EmptyBlueprint(_field)));
            }
        } else {
            setResult(make_UP(new AttributeFieldBlueprint(_field, _attr, stack)));
        }
    }

    virtual void visit(StringTerm & n) { visitTerm(n, true); }
    virtual void visit(SubstringTerm & n) {
        search::query::SimpleRegExpTerm re(vespalib::Regexp::make_from_substring(n.getTerm()),
                                           n.getView(), n.getId(), n.getWeight());
        visitTerm(re);
    }
    virtual void visit(SuffixTerm & n) {
        search::query::SimpleRegExpTerm re(vespalib::Regexp::make_from_suffix(n.getTerm()),
                                           n.getView(), n.getId(), n.getWeight());
        visitTerm(re);
    }
    virtual void visit(PredicateQuery &n) { visitPredicate(n); }
    virtual void visit(RegExpTerm & n) { visitTerm(n); }

    template <typename WS, typename NODE>
    void createDirectWeightedSet(WS *bp, NODE &n) {
        Blueprint::UP result(bp);
        for (size_t i = 0; i < n.getChildren().size(); ++i) {
            const search::query::Node &node = *n.getChildren()[i];
            vespalib::string term = search::queryeval::termAsString(node);
            uint32_t weight = search::queryeval::getWeightFromNode(node).percent();
            bp->addTerm(term, weight);
        }
        setResult(std::move(result));
    }

    template <typename WS, typename NODE>
    void createShallowWeightedSet(WS *bp, NODE &n, const FieldSpec &fs) {
        Blueprint::UP result(bp);
        for (size_t i = 0; i < n.getChildren().size(); ++i) {
            const search::query::Node &node = *n.getChildren()[i];
            uint32_t weight = search::queryeval::getWeightFromNode(node).percent();
            const string stack = StackDumpCreator::create(node);
            FieldSpec childfs = bp->getNextChildField(fs);
            bp->addTerm(make_UP(new AttributeFieldBlueprint(childfs, _attr, stack)), weight);
        }
        setResult(std::move(result));
    }

    virtual void visit(search::query::WeightedSetTerm &n) {
        bool isSingleValue = !_attr.hasMultiValue();
        bool isString = (_attr.isStringType() && _attr.hasEnum());
        bool isInteger = _attr.isIntegerType();
        if (isSingleValue && (isString || isInteger)) {
            AttributeWeightedSetBlueprint *ws
                = new AttributeWeightedSetBlueprint(_field, _attr);
            Blueprint::UP result(ws);
            for (size_t i = 0; i < n.getChildren().size(); ++i) {
                const search::query::Node &node = *n.getChildren()[i];
                uint32_t weight = search::queryeval::getWeightFromNode(node).percent();
                vespalib::string term = search::queryeval::termAsString(node);
                search::QueryTermSimple::UP qt;
                if (isInteger) {
                    qt.reset(new search::QueryTermSimple(term, search::QueryTermSimple::WORD));
                } else {
                    qt.reset(new search::QueryTermBase(term, search::QueryTermSimple::WORD));
                }
                ws->addToken(_attr.getSearch(std::move(qt), AttributeVector::SearchContext::Params()), weight);
            }
            setResult(std::move(result));
        } else {
            if (_dwa != nullptr) {
                auto *bp = new DirectWeightedSetBlueprint<queryeval::WeightedSetTermSearch>(_field, *_dwa, n.getChildren().size());
                createDirectWeightedSet(bp, n);
            } else {
                auto *bp = new WeightedSetTermBlueprint(_field);
                createShallowWeightedSet(bp, n, _field);
            }
        }
    }

    virtual void visit(search::query::DotProduct &n) {
        if (_dwa != nullptr) {
            auto *bp = new DirectWeightedSetBlueprint<queryeval::DotProductSearch>(_field, *_dwa, n.getChildren().size());
            createDirectWeightedSet(bp, n);
        } else {
            auto *bp = new DotProductBlueprint(_field);
            createShallowWeightedSet(bp, n, _field);
        }
    }

    virtual void visit(search::query::WandTerm &n) {
        if (_dwa != nullptr) {
            auto *bp = new DirectWandBlueprint(_field, *_dwa,
                                               n.getTargetNumHits(), n.getScoreThreshold(), n.getThresholdBoostFactor(),
                                               n.getChildren().size());
            createDirectWeightedSet(bp, n);
        } else {
            auto *bp = new ParallelWeakAndBlueprint(_field,
                    n.getTargetNumHits(),
                    n.getScoreThreshold(),
                    n.getThresholdBoostFactor());
            createShallowWeightedSet(bp, n, _field);
        }
    }
};

} // namespace

//-----------------------------------------------------------------------------

Blueprint::UP
AttributeBlueprintFactory::createBlueprint(const IRequestContext & requestContext,
                                            const FieldSpec &field,
                                            const search::query::Node &term)
{
    const AttributeVector * attr(requestContext.getAttribute(field.getName()));
    CreateBlueprintVisitor visitor(*this, requestContext, field, *attr);
    const_cast<Node &>(term).accept(visitor);
    return visitor.getResult();
}

}  // namespace search
