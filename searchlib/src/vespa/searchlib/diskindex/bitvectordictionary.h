// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/common/bitvector.h>
#include <vespa/searchlib/index/bitvectorkeys.h>
#include <vespa/searchlib/common/tunefileinfo.h>
#include <vespa/vespalib/stllike/string.h>
#include "bitvectorkeyscope.h"

namespace search {

namespace diskindex {

/**
 * This dictionary provides a sparse mapping from word number -> BitVector.
 * The dictionary is constructed based on the boolocc idx file and
 * the actual bit vectors are stored in the boolocc dat file.
 **/
class BitVectorDictionary
{
private:
    BitVectorDictionary(const BitVectorDictionary &rhs);

    BitVectorDictionary &
    operator=(const BitVectorDictionary &rhs);

    typedef search::index::BitVectorWordSingleKey WordSingleKey;

    uint32_t                   _docIdLimit;
    std::vector<WordSingleKey> _entries;
    size_t                     _vectorSize;
    std::unique_ptr<FastOS_File> _datFile;
    uint32_t                   _datHeaderLen;

public:
    typedef std::shared_ptr<BitVectorDictionary> SP;

    BitVectorDictionary();
    ~BitVectorDictionary();

    /**
     * Open this dictionary using the following path prefix to where
     * the files are located.  The boolocc idx file is loaded into
     * memory while the dat file is just opened.
     *
     * @param pathPrefix the path prefix to where the boolocc files
     *                   are located.
     * @return true if the files could be opened.
     **/
    bool
    open(const vespalib::string &pathPrefix,
         const TuneFileRandRead &tuneFileRead,
         BitVectorKeyScope scope);

    /**
     * Lookup the given word number and load and return the associated
     * bit vector if found.
     *
     * @param wordNum the word number to lookup a bit vector for.
     * @return the loaded bit vector or NULL if not found.
     **/
    BitVector::UP
    lookup(uint64_t wordNum);

    uint32_t
    getDocIdLimit() const
    {
        return _docIdLimit;
    }

    const std::vector<WordSingleKey> &
    getEntries() const
    {
        return _entries;
    }
};

} // namespace diskindex

} // namespace search

