// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
#include "bitvectorkeyscope.h"
#include <vespa/vespalib/objects/nbostream.h>


using search::diskindex::BitVectorKeyScope;

namespace search
{

namespace diskindex
{

const char *getBitVectorKeyScopeSuffix(BitVectorKeyScope scope)
{
    switch (scope) {
    case BitVectorKeyScope::SHARED_WORDS:
        return ".bidx";
    default:
        return ".idx";
    }
}

}

}


namespace {

uint8_t
getVal(BitVectorKeyScope scope)
{
    switch (scope) {
    case BitVectorKeyScope::SHARED_WORDS:
        return 0u;
    default:
        return 1u;
    }
}


const BitVectorKeyScope scopes[] = { BitVectorKeyScope::SHARED_WORDS,
                                     BitVectorKeyScope::PERFIELD_WORDS };

}


namespace vespalib
{

nbostream &
operator<<(nbostream &stream, const BitVectorKeyScope &scope)
{
    uint8_t val = getVal(scope);
    stream << val;
    return stream;
}

nbostream &
operator>>(nbostream &stream, BitVectorKeyScope &scope)
{
    uint8_t val;
    stream >> val;
    assert(val < sizeof(scopes) / sizeof(scopes[0]));
    scope = scopes[val];
    return stream;
}

}
