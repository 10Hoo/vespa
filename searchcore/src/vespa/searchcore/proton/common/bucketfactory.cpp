// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".proton.common.bucketfactory");

#include "bucketfactory.h"
#include <persistence/spi/types.h>

using document::BucketId;
using document::DocumentId;
using storage::spi::Bucket;
using storage::spi::PartitionId;

namespace proton {

BucketId
BucketFactory::getBucketId(const DocumentId &docId)
{
    BucketId bId = docId.getGlobalId().convertToBucketId();
    bId.setUsedBits(getNumBucketBits());
    return bId;
}


Bucket
BucketFactory::getBucket(const DocumentId &docId)
{
    return Bucket(getBucketId(docId), PartitionId(0));
}


} // namespace proton
