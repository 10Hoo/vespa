// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "chunks.h"
#include <vespa/vespalib/util/stringfmt.h>
#include <vespa/vespalib/util/crc.h>
#include <vespa/vespalib/xxhash/xxhash.h>
#include <cassert>

using std::runtime_error;
using std::make_unique;
using vespalib::make_string;
using vespalib::nbostream_longlivedbuf;

namespace search::transactionlog {

Encoding::Encoding(Crc crc, Compression compression)
    : _raw(crc | (compression >> 2))
{
    assert(crc <= Crc::xxh64);
    assert(compression <= Compression::lz4);
}

IChunk::~IChunk() = default;

void
IChunk::add(const Packet::Entry & entry) {
    _entries.emplace_back(entry);
}

void
IChunk::encode(nbostream & ) {

}

void
IChunk::decode(nbostream & is) {
    onDecode(is);
}

IChunk::UP
IChunk::create(uint8_t chunkType) {
    Encoding encoding(chunkType);
    if (encoding.getCrc() == Encoding::Crc::xxh64) {
        if (encoding.getCompression() == Encoding::Compression::none) {
            return make_unique<XXH64None>();
        } else if (encoding.getCompression() == Encoding::Compression::lz4) {
            return make_unique<XXH64LZ4>();
        } else {
            throw runtime_error(make_string("Unhandled compression type '%d'", encoding.getCompression()));
        }
    } else if (encoding.getCrc() == Encoding::Crc::ccitt_crc32) {
        if (encoding.getCompression() == Encoding::Compression::none) {
            return make_unique<CCITTCRC32None>();
        } else {
            throw runtime_error(make_string("Unhandled compression type '%d'", encoding.getCompression()));
        }
    } else {
        throw runtime_error(make_string("Unhandled crc type '%d'", encoding.getCrc()));
    }
}

int32_t Encoding::calcCrc(Crc version, const void * buf, size_t sz)
{
    if (version == xxh64) {
        return static_cast<int32_t>(XXH64(buf, sz, 0ll));
    } else if (version == ccitt_crc32) {
        vespalib::crc_32_type calculator;
        calculator.process_bytes(buf, sz);
        return calculator.checksum();
    } else {
        abort();
    }
}

}
