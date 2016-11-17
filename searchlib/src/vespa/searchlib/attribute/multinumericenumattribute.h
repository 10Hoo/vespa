// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchlib/attribute/multienumattribute.h>
#include <vespa/searchlib/attribute/attributeiterators.h>
#include <vespa/searchlib/queryeval/emptysearch.h>
#include <vespa/searchlib/attribute/numericbase.h>

namespace search {

/*
 * Implementation of multi value numeric attribute that uses an underlying enum store
 * to store unique numeric values and a multi value mapping to store enum indices for each document.
 * This class is used for both array and weighted set types.
 *
 * B: EnumAttribute<BaseClass>
 * M: MultiValueType (MultiValueMapping template argument)
 */
template <typename B, typename M>
class MultiValueNumericEnumAttribute : public MultiValueEnumAttribute<B, M>
{
protected:
    typedef typename B::BaseClass::DocId           DocId;
    typedef typename B::BaseClass::EnumHandle      EnumHandle;
public:
    typedef typename B::BaseClass::BaseType        T;
protected:
    typedef typename B::BaseClass::largeint_t      largeint_t;
    typedef typename B::BaseClass::LoadedNumericValueT LoadedNumericValueT;
    typedef typename B::BaseClass::LoadedVector    LoadedVector;
    typedef SequentialReadModifyWriteVector<LoadedNumericValueT> LoadedVectorR;
    typedef typename B::BaseClass::Weighted        Weighted;
    typedef typename B::BaseClass::WeightedInt     WeightedInt;
    typedef typename B::BaseClass::WeightedFloat   WeightedFloat;
    typedef typename B::BaseClass::WeightedEnum    WeightedEnum;

    typedef typename MultiValueEnumAttribute<B, M>::MultiValueType WeightedIndex;
    using WeightedIndexArrayRef = typename MultiValueEnumAttribute<B, M>::MultiValueArrayRef;
    typedef attribute::LoadedEnumAttribute         LoadedEnumAttribute;
    typedef attribute::LoadedEnumAttributeVector   LoadedEnumAttributeVector;
    typedef EnumStoreBase::IndexVector             EnumIndexVector;
    typedef EnumStoreBase::EnumVector              EnumVector;
    typedef EnumStoreBase::Index                   EnumIndex;

protected:
    /*
     * Specialization of SearchContext for weighted set type
     */
    class SetSearchContext : public NumericAttribute::Range<T>, public AttributeVector::SearchContext
    {
    protected:
        const MultiValueNumericEnumAttribute<B, M> & _toBeSearched;

        virtual bool
        onCmp(DocId docId, int32_t & weight) const
        {
            return cmp(docId, weight);
        }

        virtual bool
        onCmp(DocId docId) const
        {
            return cmp(docId);
        }

        virtual bool valid() const { return this->isValid(); }

    public:
        SetSearchContext(QueryTermSimple::UP qTerm, const NumericAttribute & toBeSearched) :
            NumericAttribute::Range<T>(*qTerm),
            SearchContext(toBeSearched),
            _toBeSearched(static_cast<const MultiValueNumericEnumAttribute<B, M> &>(toBeSearched))
        {
        }

        bool
        cmp(DocId doc, int32_t & weight) const
        {
            WeightedIndexArrayRef indices(_toBeSearched._mvMapping.get(doc));
            for (const WeightedIndex &wi : indices) {
                T v = _toBeSearched._enumStore.getValue(wi.value());
                if (this->match(v)) {
                    weight = wi.weight();
                    return true;
                }
            }
            return false;
        }

        bool
        cmp(DocId doc) const
        {
            WeightedIndexArrayRef indices(_toBeSearched._mvMapping.get(doc));
            for (const WeightedIndex &wi : indices) {
                T v = _toBeSearched._enumStore.getValue(wi.value());
                if (this->match(v)) {
                    return true;
                }
            }
            return false;
        }
        virtual Int64Range getAsIntegerTerm() const {
            return this->getRange();
        }

        virtual std::unique_ptr<queryeval::SearchIterator>
        createFilterIterator(fef::TermFieldMatchData * matchData, bool strict)
        {
            if (!valid()) {
                return queryeval::SearchIterator::UP(
                        new queryeval::EmptySearch());
            }
            if (getIsFilter()) {
                return queryeval::SearchIterator::UP
                    (strict
                     ? new FilterAttributeIteratorStrict<SetSearchContext>(*this, matchData)
                     : new FilterAttributeIteratorT<SetSearchContext>(*this, matchData));
            }
            return queryeval::SearchIterator::UP
                (strict
                 ? new AttributeIteratorStrict<SetSearchContext>(*this, matchData)
                 : new AttributeIteratorT<SetSearchContext>(*this, matchData));
        }
    };

    /*
     * Specialization of SearchContext for array type
     */
    class ArraySearchContext : public NumericAttribute::Range<T>, public AttributeVector::SearchContext
    {
    protected:
        const MultiValueNumericEnumAttribute<B, M> & _toBeSearched;

        virtual bool
        onCmp(DocId docId, int32_t & weight) const
        {
            return cmp(docId, weight);
        }

        virtual bool
        onCmp(DocId docId) const
        {
            return cmp(docId);
        }

        virtual bool valid() const { return this->isValid(); }

    public:
        ArraySearchContext(QueryTermSimple::UP qTerm, const NumericAttribute & toBeSearched) :
            NumericAttribute::Range<T>(*qTerm),
            SearchContext(toBeSearched),
            _toBeSearched(static_cast<const MultiValueNumericEnumAttribute<B, M> &>(toBeSearched))
        {
        }

        virtual Int64Range getAsIntegerTerm() const {
            return this->getRange();
        }

        bool
        cmp(DocId doc, int32_t & weight) const
        {
            uint32_t hitCount = 0;
            WeightedIndexArrayRef indices(_toBeSearched._mvMapping.get(doc));
            for (const WeightedIndex &wi : indices) {
                T v = _toBeSearched._enumStore.getValue(wi.value());
                if (this->match(v)) {
                    hitCount++;
                }
            }
            weight = hitCount;

            return hitCount != 0;
        }

        bool
        cmp(DocId doc) const
        {
            WeightedIndexArrayRef indices(_toBeSearched._mvMapping.get(doc));
            for (const WeightedIndex &wi : indices) {
                T v = _toBeSearched._enumStore.getValue(wi.value());
                if (this->match(v)) {
                    return true;
                }
            }

            return false;
        }

        virtual std::unique_ptr<queryeval::SearchIterator>
        createFilterIterator(fef::TermFieldMatchData * matchData, bool strict)
        {
            if (!valid()) {
                return queryeval::SearchIterator::UP(
                        new queryeval::EmptySearch());
            }
            if (getIsFilter()) {
                return queryeval::SearchIterator::UP
                    (strict
                     ? new FilterAttributeIteratorStrict<ArraySearchContext>(*this, matchData)
                     : new FilterAttributeIteratorT<ArraySearchContext>(*this, matchData));
            }
            return queryeval::SearchIterator::UP
                (strict
                 ? new AttributeIteratorStrict<ArraySearchContext>(*this, matchData)
                 : new AttributeIteratorT<ArraySearchContext>(*this, matchData));
        }
    };


public:
    MultiValueNumericEnumAttribute(const vespalib::string & baseFileName, const AttributeVector::Config & cfg);

    virtual bool onLoad();

    bool
    onLoadEnumerated(typename B::ReaderBase &attrReader);

    AttributeVector::SearchContext::UP
    getSearch(QueryTermSimple::UP term, const AttributeVector::SearchContext::Params & params) const override;

    //-------------------------------------------------------------------------
    // Attribute read API
    //-------------------------------------------------------------------------
    virtual T get(DocId doc) const {
        WeightedIndexArrayRef indices(this->_mvMapping.get(doc));
        if (indices.size() == 0) {
            return T();
        } else {
            return this->_enumStore.getValue(indices[0].value());
        }
    }
    virtual largeint_t getInt(DocId doc) const {
        return static_cast<largeint_t>(get(doc));
    }
    virtual double getFloat(DocId doc) const {
        return static_cast<double>(get(doc));
    }

    template <typename BufferType>
    uint32_t getHelper(DocId doc, BufferType * buffer, uint32_t sz) const {
        WeightedIndexArrayRef indices(this->_mvMapping.get(doc));
        uint32_t valueCount = indices.size();
        for(uint32_t i = 0, m = std::min(sz, valueCount); i < m; i++) {
            buffer[i] = static_cast<BufferType>(this->_enumStore.getValue(indices[i].value()));
        }
        return valueCount;
    }
    virtual uint32_t getAll(DocId doc, T * v, uint32_t sz) const {
        return getHelper(doc, v, sz);
    }
    virtual uint32_t get(DocId doc, largeint_t * v, uint32_t sz) const {
        return getHelper(doc, v, sz);
    }
    virtual uint32_t get(DocId doc, double * v, uint32_t sz) const {
        return getHelper(doc, v, sz);
    }

    template <typename WeightedType, typename ValueType>
    uint32_t getWeightedHelper(DocId doc, WeightedType * buffer, uint32_t sz) const {
        WeightedIndexArrayRef indices(this->_mvMapping.get(doc));
        uint32_t valueCount = indices.size();
        for (uint32_t i = 0, m = std::min(sz, valueCount); i < m; ++i) {
            buffer[i] = WeightedType(static_cast<ValueType>(this->_enumStore.getValue(indices[i].value())), indices[i].weight());
        }
        return valueCount;
    }
    virtual uint32_t getAll(DocId doc, Weighted * v, uint32_t sz) const {
        return getWeightedHelper<Weighted, T>(doc, v, sz);
    }
    virtual uint32_t get(DocId doc, WeightedInt * v, uint32_t sz) const {
        return getWeightedHelper<WeightedInt, largeint_t>(doc, v, sz);
    }
    virtual uint32_t get(DocId doc, WeightedFloat * v, uint32_t sz) const {
        return getWeightedHelper<WeightedFloat, double>(doc, v, sz);
    }

private:
    typedef typename B::template PrimitiveReader<typename B::LoadedValueType> AttributeReader;
    void loadAllAtOnce(AttributeReader & attrReader, size_t numDocs, size_t numValues);
};

} // namespace search

