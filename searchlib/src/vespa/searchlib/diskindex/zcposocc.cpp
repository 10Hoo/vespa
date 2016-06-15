// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
#include "zcposocc.h"
#include <vespa/searchlib/index/postinglistcounts.h>
#include <vespa/searchlib/index/postinglistcountfile.h>
#include <vespa/searchlib/index/postinglistfile.h>
#include <vespa/searchlib/index/docidandfeatures.h>

LOG_SETUP(".diskindex.zcposocc");

namespace search
{

namespace diskindex
{

using search::bitcompression::PosOccFieldsParams;
using search::bitcompression::EG2PosOccDecodeContext;
using search::bitcompression::EGPosOccDecodeContext;
using search::index::PostingListCountFileSeqRead;
using search::index::PostingListCountFileSeqWrite;

Zc4PosOccSeqRead::Zc4PosOccSeqRead(PostingListCountFileSeqRead *countFile)
    : Zc4PostingSeqRead(countFile),
      _fieldsParams(),
      _cookedDecodeContext(&_fieldsParams),
      _rawDecodeContext(&_fieldsParams)
{
    _decodeContext = &_cookedDecodeContext;
    _decodeContext->setReadContext(&_readContext);
    _readContext.setDecodeContext(_decodeContext);
}


void
Zc4PosOccSeqRead::
setFeatureParams(const PostingListParams &params)
{
    bool oldCooked = _decodeContext == &_cookedDecodeContext;
    bool newCooked = oldCooked;
    params.get("cooked", newCooked);
    if (oldCooked != newCooked) {
        if (newCooked) {
            _cookedDecodeContext = _rawDecodeContext;
            _decodeContext = &_cookedDecodeContext;
        } else {
            _rawDecodeContext = _cookedDecodeContext;
            _decodeContext = &_rawDecodeContext;
        }
        _readContext.setDecodeContext(_decodeContext);
    }
}


const vespalib::string &
Zc4PosOccSeqRead::getSubIdentifier(void)
{
    PosOccFieldsParams fieldsParams;
    EG2PosOccDecodeContext<true> d(&fieldsParams);
    return d.getIdentifier();
}


Zc4PosOccSeqWrite::Zc4PosOccSeqWrite(const Schema &schema,
                                     uint32_t indexId,
                                     PostingListCountFileSeqWrite *countFile)
    : Zc4PostingSeqWrite(countFile),
      _fieldsParams(),
      _realEncodeFeatures(&_fieldsParams)
{
    _encodeFeatures = &_realEncodeFeatures;
    _encodeFeatures->setWriteContext(&_featureWriteContext);
    _featureWriteContext.setEncodeContext(_encodeFeatures);
    _fieldsParams.setSchemaParams(schema, indexId);
}


ZcPosOccSeqRead::ZcPosOccSeqRead(PostingListCountFileSeqRead *countFile)
    : ZcPostingSeqRead(countFile),
      _fieldsParams(),
      _cookedDecodeContext(&_fieldsParams),
      _rawDecodeContext(&_fieldsParams)
{
    _decodeContext = &_cookedDecodeContext;
    _decodeContext->setReadContext(&_readContext);
    _readContext.setDecodeContext(_decodeContext);
}


void
ZcPosOccSeqRead::
setFeatureParams(const PostingListParams &params)
{
    bool oldCooked = _decodeContext == &_cookedDecodeContext;
    bool newCooked = oldCooked;
    params.get("cooked", newCooked);
    if (oldCooked != newCooked) {
        if (newCooked) {
            _cookedDecodeContext = _rawDecodeContext;
            _decodeContext = &_cookedDecodeContext;
        } else {
            _rawDecodeContext = _cookedDecodeContext;
            _decodeContext = &_rawDecodeContext;
        }
        _readContext.setDecodeContext(_decodeContext);
    }
}


const vespalib::string &
ZcPosOccSeqRead::getSubIdentifier(void)
{
    PosOccFieldsParams fieldsParams;
    EGPosOccDecodeContext<true> d(&fieldsParams);
    return d.getIdentifier();
}


ZcPosOccSeqWrite::ZcPosOccSeqWrite(const Schema &schema,
                                   uint32_t indexId,
                                   PostingListCountFileSeqWrite *countFile)
    : ZcPostingSeqWrite(countFile),
      _fieldsParams(),
      _realEncodeFeatures(&_fieldsParams)
{
    _encodeFeatures = &_realEncodeFeatures;
    _encodeFeatures->setWriteContext(&_featureWriteContext);
    _featureWriteContext.setEncodeContext(_encodeFeatures);
    _fieldsParams.setSchemaParams(schema, indexId);
}


} // namespace diskindex

} // namespace search
