// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "bucket_db_owner.h"

namespace proton
{

namespace bucketdb
{

/**
 * Base class for split/join handling utility classes that bundles temporary
 * variables used during the operation.
 */
class BucketSessionBase
{
public:
    typedef document::GlobalId GlobalId;
    typedef document::BucketId BucketId;
    typedef storage::spi::Timestamp Timestamp;

protected:
    BucketDBOwner::Guard _bucketDB;

public:
    BucketSessionBase(BucketDBOwner &bucketDB);

    bool
    extractInfo(const BucketId &bucket, BucketState *&info);

    static bool
    calcFixupNeed(BucketState *state, bool wantActive, bool fixup);
};


}

}
