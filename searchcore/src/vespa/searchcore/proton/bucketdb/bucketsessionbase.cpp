// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "bucketdb.h"
#include "bucketsessionbase.h"

namespace proton
{

namespace bucketdb
{

BucketSessionBase::BucketSessionBase(BucketDBOwner &bucketDB)
    : _bucketDB(bucketDB.takeGuard())
{
}


bool
BucketSessionBase::extractInfo(const BucketId &bucket, BucketState *&state)
{
    if (bucket.valid()) {
        state = _bucketDB->getBucketStatePtr(bucket);
    }
    return state && state->isActive();
}


bool
BucketSessionBase::calcFixupNeed(BucketState *state, bool wantActive,
                                 bool fixup)
{
    if (state && state->isActive() != wantActive) {
        if (fixup) {
            state->setActive(wantActive);
        }
        return state->getReadyCount() != 0;
    }
    return false;
}


}

}
