// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchlib/attribute/attributevector.h>
#include <vespa/searchlib/util/foldedstringcompare.h>
#include <vespa/vespalib/text/utf8.h>
#include <vespa/vespalib/util/regexp.h>
#include <vespa/vespalib/text/lowercase.h>
#include <vespa/searchlib/attribute/enumstorebase.h>
#include <vespa/searchlib/attribute/loadedenumvalue.h>
#include <vespa/searchlib/attribute/loadedstringvalue.h>
#include <vespa/searchlib/attribute/changevector.h>

namespace search {

class StringEntryType;

class StringAttribute : public AttributeVector
{
public:
    typedef vespalib::Array<uint32_t> OffsetVector;
    typedef const char *                  LoadedValueType;
    typedef EnumStoreBase::Index          EnumIndex;
    typedef EnumStoreBase::IndexVector    EnumIndexVector;
    typedef EnumStoreBase::EnumVector     EnumVector;
    typedef attribute::LoadedStringVector LoadedVector;
public:
    DECLARE_IDENTIFIABLE_ABSTRACT(StringAttribute);
    bool append(DocId doc, const vespalib::string & v, int32_t weight) {
        return AttributeVector::append(_changes, doc, StringChangeData(v), weight);
    }
    template<typename Accessor>
    bool append(DocId doc, Accessor & ac) {
        return AttributeVector::append(_changes, doc, ac);
    }
    bool remove(DocId doc, const vespalib::string & v, int32_t weight) {
        return AttributeVector::remove(_changes, doc, StringChangeData(v), weight);
    }
    bool update(DocId doc, const vespalib::string & v) {
        return AttributeVector::update(_changes, doc, StringChangeData(v));
    }
    bool apply(DocId doc, const ArithmeticValueUpdate & op);
    virtual bool applyWeight(DocId doc, const FieldValue & fv, const ArithmeticValueUpdate & wAdjust);
    virtual bool findEnum(const char * value, EnumHandle & e) const = 0;
    virtual uint32_t get(DocId doc, largeint_t * v, uint32_t sz) const;
    virtual uint32_t get(DocId doc, double * v, uint32_t sz) const;
    virtual uint32_t get(DocId doc, WeightedInt * v, uint32_t sz) const;
    virtual uint32_t get(DocId doc, WeightedFloat * v, uint32_t sz) const;
    virtual const char *get(DocId doc) const = 0;
    virtual uint32_t clearDoc(DocId doc);
    virtual largeint_t getDefaultValue() const { return 0; }
    static size_t countZero(const char * bt, size_t sz);
    static void generateOffsets(const char * bt, size_t sz, OffsetVector & offsets);
    virtual const char * getFromEnum(EnumHandle e) const = 0;

protected:
    StringAttribute(const vespalib::string & name);
    StringAttribute(const vespalib::string & name, const Config & c);
    static const char * defaultValue() { return ""; }
    typedef ChangeTemplate<StringChangeData> Change;
    typedef ChangeVectorT< Change > ChangeVector;
    typedef StringEntryType EnumEntryType;
    ChangeVector _changes;
    Change _defaultValue;
    virtual bool onLoad();

    bool onLoadEnumerated(ReaderBase &attrReader);

    virtual bool
    onAddDoc(DocId doc);
private:
    typedef attribute::LoadedStringVectorReal LoadedVectorR;
    virtual void fillPostings(LoadedVector & loaded);
    virtual void fillEnum(LoadedVector & loaded);
    virtual void fillValues(LoadedVector & loaded);

    virtual void
    fillEnum0(const void *src,
              size_t srcLen,
              EnumIndexVector &eidxs);

    virtual void
    fillEnumIdx(ReaderBase &attrReader,
                const EnumIndexVector &eidxs,
                attribute::LoadedEnumAttributeVector &loaded);

    virtual void
    fillEnumIdx(ReaderBase &attrReader,
                const EnumIndexVector &eidxs,
                EnumVector &enumHist);

    virtual void
    fillPostingsFixupEnum(const attribute::LoadedEnumAttributeVector &loaded);

    virtual void
    fixupEnumRefCounts(const EnumVector &enumHist);

    virtual largeint_t getInt(DocId doc)  const { return strtoll(get(doc), NULL, 0); }
    virtual double getFloat(DocId doc)    const { return strtod(get(doc), NULL); }
    virtual const char * getString(DocId doc, char * v, size_t sz) const { (void) v; (void) sz; return get(doc); }

    virtual long onSerializeForAscendingSort(DocId doc, void * serTo, long available, const common::BlobConverter * bc) const;
    virtual long onSerializeForDescendingSort(DocId doc, void * serTo, long available, const common::BlobConverter * bc) const;

    template <typename T>
    void loadAllAtOnce(T & loaded, FileUtil::LoadedBuffer::UP dataBuffer, uint32_t numDocs, ReaderBase & attrReader, bool hasWeight, bool hasIdx);

    class StringSearchContext : public SearchContext {
    public:
        StringSearchContext(QueryTermSimple::UP qTerm, const StringAttribute & toBeSearched);
        virtual ~StringSearchContext();
    private:
        bool                        _isPrefix;
        bool                        _isRegex;
    protected:
        bool valid() const override {
            return (_queryTerm.get() && (!_queryTerm->empty()));
        }

        const QueryTermBase & queryTerm() const override {
            return static_cast<const QueryTermBase &>(*_queryTerm);
        }
        bool isMatch(const char *src) const {
            if (__builtin_expect(isRegex(), false)) {
                return getRegex()->match(src);
            }
            vespalib::Utf8ReaderForZTS u8reader(src);
            uint32_t j = 0;
            uint32_t val;
            for (;; ++j) {
                val = u8reader.getChar();
                val = vespalib::LowerCase::convert(val);
                if (_termUCS4[j] == 0 || _termUCS4[j] != val) {
                    break;
                }
            }
            return (_termUCS4[j] == 0 && (val == 0 || isPrefix()));
        }
        class CollectHitCount {
        public:
            CollectHitCount() : _hitCount(0) { }
            void addWeight(int32_t w) {
                (void) w;
                _hitCount++;
            }
            int32_t getWeight() const { return _hitCount; }
            bool hasMatch() const { return _hitCount != 0; }
        private:
            uint32_t _hitCount;
        };
        class CollectWeight {
        public:
            CollectWeight() : _hitCount(0), _weight(0) { }
            void addWeight(int32_t w) {
                _weight += w;
                _hitCount++;
            }
            int32_t getWeight() const { return _weight; }
            bool hasMatch() const { return _hitCount != 0; }
        private:
            uint32_t _hitCount;
            int32_t  _weight;
        };

        template<typename WeightedT, typename Accessor, typename Collector>
        void collectMatches(vespalib::ConstArrayRef<WeightedT> w, const Accessor & ac, Collector & collector) const {
            for (const WeightedT &wRef : w) {
                if (isMatch(ac.get(wRef.value()))) {
                    collector.addWeight(wRef.weight());
                }
            }
        }


        bool onCmp(DocId docId, int32_t & weight) const override;
        bool onCmp(DocId docId) const override;

        bool isPrefix() const { return _isPrefix; }
        bool  isRegex() const { return _isRegex; }
        QueryTermSimple::UP         _queryTerm;
        const ucs4_t              * _termUCS4;
        const vespalib::Regexp * getRegex() const { return _regex.get(); }
    private:
        WeightedConstChar * getBuffer() const {
            if (_buffer == NULL) {
                _buffer = new WeightedConstChar[_bufferLen];
            }
            return _buffer;
        }
        unsigned                    _bufferLen;
        mutable WeightedConstChar * _buffer;
        std::unique_ptr<vespalib::Regexp>   _regex;
    };
    SearchContext::UP getSearch(QueryTermSimple::UP term, const SearchContext::Params & params) const override;
};

}

