// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.


#include "attribute_weighted_set_blueprint.h"
#include <vespa/searchlib/queryeval/weighted_set_term_search.h>

namespace search {

namespace {

//-----------------------------------------------------------------------------

class UseAttr
{
private:
    const attribute::IAttributeVector &_attr;

protected:
    const attribute::IAttributeVector &attribute() const { return _attr; }

public:
    UseAttr(const attribute::IAttributeVector & attr)
        : _attr(attr) {}
};

//-----------------------------------------------------------------------------

class UseStringEnum : public UseAttr
{
public:
    UseStringEnum(const AttributeVector & attr)
        : UseAttr(attr) {}
    bool mapToken(const AttributeVector::SearchContext &context,
                  int64_t &token) const
    {
        attribute::IAttributeVector::EnumHandle handle;
        if (attribute().findEnum(context.queryTerm().getTerm(), handle)) {
            token = handle;
            return true;
        }
        return false;
    }
    int64_t getToken(uint32_t docId) const {
        return attribute().getEnum(docId);
    }
};

//-----------------------------------------------------------------------------

class UseInteger : public UseAttr
{
public:
    UseInteger(const AttributeVector & attr) : UseAttr(attr) {}
    bool mapToken(const AttributeVector::SearchContext &context,
                  int64_t &token) const
    {
        Int64Range range(context.getAsIntegerTerm());
        if (range.isPoint()) {
            token = range.lower();
            return true;
        }
        return false;
    }
    int64_t getToken(uint32_t docId) const {
        return attribute().getInt(docId);
    }
};

//-----------------------------------------------------------------------------

template <typename T>
class AttributeFilter : public queryeval::SearchIterator
{
private:
    typedef vespalib::hash_map<int64_t, int32_t> Map;
    typedef fef::TermFieldMatchData TFMD;

    TFMD    &_tfmd;
    T        _attr;
    Map      _map;
    int32_t  _weight;

public:
    AttributeFilter(fef::TermFieldMatchData &tfmd,
                    const AttributeVector & attr,
                    const std::vector<int32_t> weights,
                    const std::vector<AttributeVector::SearchContext*> contexts)
        : _tfmd(tfmd), _attr(attr), _map(), _weight(0)
    {
        for (size_t i = 0; i < contexts.size(); ++i) {
            int64_t token(0);
            if (_attr.mapToken(*contexts[i], token)) {
                _map[token] = weights[i];
            }
        }
    }
    virtual void doSeek(uint32_t docId) {
        Map::const_iterator pos = _map.find(_attr.getToken(docId));
        if (pos != _map.end()) {
            _weight = pos->second;
            setDocId(docId);
        }
    }
    virtual void doUnpack(uint32_t docId) {
        _tfmd.reset(docId);
        fef::TermFieldMatchDataPosition pos;
        pos.setElementWeight(_weight);
        _tfmd.appendPosition(pos);
    }
    virtual void visitMembers(vespalib::ObjectVisitor &) const {}
};

//-----------------------------------------------------------------------------

} // namespace search::<unnamed>

AttributeWeightedSetBlueprint::AttributeWeightedSetBlueprint(const queryeval::FieldSpec &field, const AttributeVector & attr)
    : queryeval::ComplexLeafBlueprint(field),
      _numDocs(attr.getNumDocs()),
      _estHits(0),
      _weights(),
      _attr(attr),
      _contexts()
{
}

AttributeWeightedSetBlueprint::~AttributeWeightedSetBlueprint()
{
    while (!_contexts.empty()) {
        delete _contexts.back();
        _contexts.pop_back();
    }
}

void
AttributeWeightedSetBlueprint::addToken(AttributeVector::SearchContext::UP context, int32_t weight)
{
    _estHits = std::min(_estHits + context->approximateHits(), _numDocs);
    setEstimate(HitEstimate(_estHits, (_estHits == 0)));
    _weights.push_back(weight);
    _contexts.push_back(context.get());
    context.release();
}

queryeval::SearchIterator::UP
AttributeWeightedSetBlueprint::createLeafSearch(const fef::TermFieldMatchDataArray &tfmda,
                                                bool strict) const
{
    assert(tfmda.size() == 1);
    fef::TermFieldMatchData &tfmd = *tfmda[0];
    if (strict) { // use generic weighted set search
        std::vector<queryeval::SearchIterator*> children(_contexts.size());
        for (size_t i = 0; i < _contexts.size(); ++i) {
            children[i] = _contexts[i]->createIterator(&tfmd,
                                                       true).release();
        }
        return queryeval::SearchIterator::UP(queryeval::WeightedSetTermSearch::create(children, tfmd, _weights));
    } else { // use attribute filter optimization
        bool isSingleValue = !_attr.hasMultiValue();
        bool isString = (_attr.isStringType() && _attr.hasEnum());
        bool isInteger = _attr.isIntegerType();
        assert(isSingleValue);
        (void) isSingleValue;
        if (isString) {
            return queryeval::SearchIterator::UP(new AttributeFilter<UseStringEnum>(tfmd, _attr, _weights, _contexts));
        } else {
            assert(isInteger);
            (void) isInteger;
            return queryeval::SearchIterator::UP(new AttributeFilter<UseInteger>(tfmd, _attr, _weights, _contexts));
        }
    }
}

void
AttributeWeightedSetBlueprint::fetchPostings(bool strict)
{
    if (strict) {
        for (size_t i = 0; i < _contexts.size(); ++i) {
            _contexts[i]->fetchPostings(true);
        }
    }
}

} // namespace search
