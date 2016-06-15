// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/storageapi/messageapi/bucketreply.h>

#include <vespa/storageapi/messageapi/bucketcommand.h>

namespace storage {
namespace api {

BucketReply::BucketReply(const BucketCommand& cmd,
                         const ReturnCode& code)
    : StorageReply(cmd, code),
      _bucket(cmd.getBucketId()),
      _originalBucket(cmd.getOriginalBucketId())
{
}

void
BucketReply::print(std::ostream& out, bool verbose,
                   const std::string& indent) const
{
    out << "BucketReply(" << _bucket;
    if (hasBeenRemapped()) {
        out << " <- " << _originalBucket;
    }
    out << ")";
    if (verbose) {
        out << " : ";
        StorageReply::print(out, verbose, indent);
    }
}

} // api
} // storage
