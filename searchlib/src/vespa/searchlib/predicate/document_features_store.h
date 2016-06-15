// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "predicate_tree_annotator.h"
#include <vespa/searchlib/btree/btree.h>
#include <vespa/searchlib/memoryindex/wordstore.h>
#include <vespa/searchlib/util/memoryusage.h>
#include <vespa/vespalib/data/databuffer.h>
#include <vespa/vespalib/stllike/hash_map.h>
#include <vespa/vespalib/util/array.h>
#include <unordered_set>

namespace search {
namespace predicate {

/**
 * Class used to track the {featureId, docId} pairs that are inserted
 * into the btree memory index dictionary. These pairs are later used
 * when removing all remains of a document from the feature posting
 * lists of the dictionary.
 */
class DocumentFeaturesStore {
    typedef memoryindex::WordStore WordStore;
    struct Range {
        btree::EntryRef label_ref;
        int64_t from;
        int64_t to;
    };
    // Compares EntryRefs by their corresponding word in a WordStore.
    // To find a word without knowing its EntryRef, set the word in
    // the constructor and search for an illegal EntryRef.
    class KeyComp {
        const WordStore &_word_store;
        const vespalib::string _word;

        const char *getWord(btree::EntryRef ref) const {
            return ref.valid() ? _word_store.getWord(ref) : _word.c_str();
        }

    public:
        KeyComp(const WordStore &word_store, const vespalib::stringref &word)
            : _word_store(word_store),
              _word(word) {
        }

        bool operator()(const btree::EntryRef &lhs,
                        const btree::EntryRef &rhs) const {
            return strcmp(getWord(lhs), getWord(rhs)) < 0;
        }
    };
    typedef vespalib::Array<uint64_t> FeatureVector;
    typedef vespalib::hash_map<uint32_t, FeatureVector> DocumentFeaturesMap;
    typedef vespalib::Array<Range> RangeVector;
    typedef vespalib::hash_map<uint32_t, RangeVector> RangeFeaturesMap;
    typedef btree::BTree<btree::EntryRef, btree::BTreeNoLeafData,
                         btree::NoAggregated, const KeyComp &> WordIndex;

    DocumentFeaturesMap _docs;
    RangeFeaturesMap _ranges;
    WordStore _word_store;
    WordIndex _word_index;
    uint32_t _currDocId;
    FeatureVector *_currFeatures;
    size_t _numFeatures;
    size_t _numRanges;
    uint32_t _arity;

    void setCurrent(uint32_t docId, FeatureVector *features);

public:
    typedef std::unordered_set<uint64_t> FeatureSet;

    DocumentFeaturesStore(uint32_t arity);
    DocumentFeaturesStore(vespalib::MMapDataBuffer &buffer);
    ~DocumentFeaturesStore();

    void insert(uint64_t featureId, uint32_t docId);
    void insert(const PredicateTreeAnnotations &annotations, uint32_t docId);
    FeatureSet get(uint32_t docId) const;
    void remove(uint32_t docId);
    search::MemoryUsage getMemoryUsage() const;

    void serialize(vespalib::MMapDataBuffer &buffer) const;
};

}  // namespace predicate
}  // namespace search

