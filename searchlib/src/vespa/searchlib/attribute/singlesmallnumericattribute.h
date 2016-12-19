// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "integerbase.h"
#include "floatbase.h"
#include "attributeiterators.hpp"
#include <vespa/searchlib/common/rcuvector.h>
#include <vespa/searchlib/query/query.h>
#include <vespa/searchlib/queryeval/emptysearch.h>
#include <limits>

namespace search {

class SingleValueSmallNumericAttribute : public IntegerAttributeTemplate<int8_t>
{
private:
//    friend class AttributeVector::SearchContext;
    typedef IntegerAttributeTemplate<int8_t> B;
    typedef B::BaseType      T;
    typedef B::DocId         DocId;
    typedef B::EnumHandle    EnumHandle;
    typedef B::largeint_t    largeint_t;
    typedef B::Weighted      Weighted;
    typedef B::WeightedInt   WeightedInt;
    typedef B::WeightedFloat WeightedFloat;
    typedef B::WeightedEnum  WeightedEnum;
    typedef B::generation_t generation_t;

protected:
    typedef uint32_t Word;	// Large enough to contain numDocs.
private:
    Word _valueMask;            // 0x01, 0x03 or 0x0f
    uint32_t _valueShiftShift;  // 0x00, 0x01 or 0x02
    uint32_t _valueShiftMask;   // 0x1f, 0x0f or 0x07
    uint32_t _wordShift;        // 0x05, 0x04 or 0x03

    typedef search::attribute::RcuVectorBase<Word> DataVector;
    DataVector _wordData;

    virtual T getFromEnum(EnumHandle e) const {
        (void) e;
        return T();
    }

protected:
    virtual bool
    findEnum(T value, EnumHandle & e) const
    {
        (void) value; (void) e;
        return false;
    }

    void
    set(DocId doc, T v)
    {
        Word &word = _wordData[doc >> _wordShift];
        uint32_t valueShift = (doc & _valueShiftMask) << _valueShiftShift;
        word = (word & ~(_valueMask << valueShift)) |
               ((v & _valueMask) << valueShift);
    }


public:
    /*
     * Specialization of SearchContext
     */
    class SingleSearchContext : public NumericAttribute::Range<T>, public SearchContext
    {
    private:
        const Word *_wordData;
        Word _valueMask;
        uint32_t _valueShiftShift;
        uint32_t _valueShiftMask;
        uint32_t _wordShift;

        bool onCmp(DocId docId, int32_t & weight) const override {
            return cmp(docId, weight);
        }

        bool onCmp(DocId docId) const override {
            return cmp(docId);
        }

        bool valid() const override;

    public:
        SingleSearchContext(QueryTermSimple::UP qTerm, const NumericAttribute & toBeSearched);

        bool cmp(DocId docId, int32_t & weight) const {
            const Word &word = _wordData[docId >> _wordShift];
            uint32_t valueShift = (docId & _valueShiftMask) << _valueShiftShift;
            T v = (word >> valueShift) & _valueMask;
            weight = 1;
            return match(v);
        }

        bool cmp(DocId docId) const {
            const Word &word = _wordData[docId >> _wordShift];
            uint32_t valueShift = (docId & _valueShiftMask) << _valueShiftShift;
            T v = (word >> valueShift) & _valueMask;
            return match(v);
        }

        Int64Range getAsIntegerTerm() const override;

        std::unique_ptr<queryeval::SearchIterator>
        createFilterIterator(fef::TermFieldMatchData * matchData, bool strict) override;
    };

    SingleValueSmallNumericAttribute(const vespalib::string & baseFileName,
                                     const Config &c,
                                     Word valueMask,
                                     uint32_t valueShiftShift,
                                     uint32_t valueShiftMask,
                                     uint32_t wordShift);

    virtual ~SingleValueSmallNumericAttribute(void);

    virtual uint32_t
    getValueCount(DocId doc) const
    {
        if (doc >= B::getNumDocs()) {
            return 0;
        }
        return 1;
    }
    virtual void onCommit();
    virtual void onUpdateStat();
    virtual void removeOldGenerations(generation_t firstUsed);
    virtual void onGenerationChange(generation_t generation);
    virtual bool addDoc(DocId & doc) {
        if ((B::getNumDocs() & _valueShiftMask) == 0) {
            bool incGen = _wordData.isFull();
            _wordData.push_back(Word());
            std::atomic_thread_fence(std::memory_order_release);
            B::incNumDocs();
            doc = B::getNumDocs() - 1;
            updateUncommittedDocIdLimit(doc);
            if (incGen) {
                this->incGeneration();
            } else
                this->removeAllOldGenerations();
        } else {
            B::incNumDocs();
            doc = B::getNumDocs() - 1;
            updateUncommittedDocIdLimit(doc);
        }
        return true;
    }
    virtual bool onLoad();

    virtual void
    onSave(IAttributeSaveTarget &saveTarget);

    SearchContext::UP
    getSearch(QueryTermSimple::UP term, const SearchContext::Params & params) const override;

    T getFast(DocId doc) const {
        const Word &word = _wordData[doc >> _wordShift];
        uint32_t valueShift = (doc & _valueShiftMask) << _valueShiftShift;
        return (word >> valueShift) & _valueMask;
    }

    //-------------------------------------------------------------------------
    // new read api
    //-------------------------------------------------------------------------
    virtual T get(DocId doc) const {
        return getFast(doc);
    }
    virtual largeint_t getInt(DocId doc) const {
        return static_cast<largeint_t>(getFast(doc));
    }
    virtual void
    getEnumValue(const EnumHandle * v, uint32_t *e, uint32_t sz) const {
        (void) v;
        (void) e;
        (void) sz;
    }
    virtual double getFloat(DocId doc) const {
        return static_cast<double>(getFast(doc));
    }
    virtual uint32_t getEnum(DocId doc) const {
        (void) doc;
        return std::numeric_limits<uint32_t>::max(); // does not have enum
    }
    virtual uint32_t getAll(DocId doc, T * v, uint32_t sz) const {
        if (sz > 0) {
            v[0] = getFast(doc);
        }
        return 1;
    }
    virtual uint32_t get(DocId doc, largeint_t * v, uint32_t sz) const {
        if (sz > 0) {
            v[0] = static_cast<largeint_t>(getFast(doc));
        }
        return 1;
    }
    virtual uint32_t get(DocId doc, double * v, uint32_t sz) const {
        if (sz > 0) {
            v[0] = static_cast<double>(getFast(doc));
        }
        return 1;
    }
    virtual uint32_t get(DocId doc, EnumHandle * e, uint32_t sz) const {
        if (sz > 0) {
            e[0] = getEnum(doc);
        }
        return 1;
    }
    virtual uint32_t getAll(DocId doc, Weighted * v, uint32_t sz) const {
        (void) doc; (void) v; (void) sz;
        return 0;
    }
    virtual uint32_t get(DocId doc, WeightedInt * v, uint32_t sz) const {
        if (sz > 0) {
            v[0] = WeightedInt(static_cast<largeint_t>(getFast(doc)));
        }
        return 1;
    }
    virtual uint32_t get(DocId doc, WeightedFloat * v, uint32_t sz) const {
        if (sz > 0) {
            v[0] = WeightedFloat(static_cast<double>(getFast(doc)));
        }
        return 1;
    }
    virtual uint32_t get(DocId doc, WeightedEnum * e, uint32_t sz) const {
        (void) doc; (void) e; (void) sz;
        return 0;
    }

    virtual void
    clearDocs(DocId lidLow, DocId lidLimit);

    virtual void
    onShrinkLidSpace();

    virtual uint64_t getEstimatedSaveByteSize() const override;
};


class SingleValueBitNumericAttribute : public SingleValueSmallNumericAttribute
{
public:
    SingleValueBitNumericAttribute(const vespalib::string & baseFileName, const search::GrowStrategy & grow);
};


class SingleValueSemiNibbleNumericAttribute : public SingleValueSmallNumericAttribute
{
public:
    SingleValueSemiNibbleNumericAttribute(const vespalib::string & baseFileName, const search::GrowStrategy & grow);
};

class SingleValueNibbleNumericAttribute : public SingleValueSmallNumericAttribute
{
public:
    SingleValueNibbleNumericAttribute(const vespalib::string & baseFileName, const search::GrowStrategy & grow);
};

}

