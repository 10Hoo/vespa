// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "iordereddocumentinserter.h"
#include "memoryfieldindex.h"
#include <limits>

namespace search
{

namespace memoryindex
{

class IDocumentInsertListener;


/**
 * Class for inserting updates to MemoryFieldIndex in an ordered manner
 * (single pass scan of dictionary tree)
 *
 * Insert order must be properly sorted, by (word, docId)
 */
class OrderedDocumentInserter : public IOrderedDocumentInserter
{
    vespalib::stringref _word;
    uint32_t _prevDocId;
    bool     _prevAdd;
    using DictionaryTree = MemoryFieldIndex::DictionaryTree;
    using PostingListStore = MemoryFieldIndex::PostingListStore;
    using KeyComp = MemoryFieldIndex::KeyComp;
    using WordKey = MemoryFieldIndex::WordKey;
    using PostingListKeyDataType = MemoryFieldIndex::PostingListKeyDataType;
    MemoryFieldIndex        &_fieldIndex;
    DictionaryTree::Iterator _dItr;
    IDocumentInsertListener &_listener;

    // Pending changes to posting list for (_word)
    std::vector<uint32_t>    _removes;
    std::vector<PostingListKeyDataType> _adds;


    static constexpr uint32_t noFieldId = std::numeric_limits<uint32_t>::max();
    static constexpr uint32_t noDocId = std::numeric_limits<uint32_t>::max();

    /*
     * Flush pending changes to postinglist for (_word).
     *
     * _dItr is located at correct position.
     */
    void flushWord();

public:
    OrderedDocumentInserter(MemoryFieldIndex &fieldIndex);
    virtual ~OrderedDocumentInserter();
    virtual void setNextWord(const vespalib::stringref word) override;
    virtual void add(uint32_t docId,
                     const index::DocIdAndFeatures &features) override;
    virtual void remove(uint32_t docId) override;

    /*
     * Flush pending changes to postinglist for (_word).  Also flush
     * insert listener.
     *
     * _dItr is located at correct position.
     */
    virtual void flush() override;

    /*
     * Rewind iterator, to start new pass.
     */
    virtual void rewind() override;

    // Used by unit test
    btree::EntryRef getWordRef() const;
};

}

}
