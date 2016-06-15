// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/storageapi/messageapi/bucketinforeply.h>

namespace storage {
namespace api {

BucketInfoReply::BucketInfoReply(const BucketInfoCommand& cmd,
                                 const ReturnCode& code)
    : BucketReply(cmd, code),
      _result()
{
}

void
BucketInfoReply::print(std::ostream& out, bool verbose,
                       const std::string& indent) const
{
    out << "BucketInfoReply(" << _result << ")";
    if (verbose) {
        out << " : ";
        BucketReply::print(out, verbose, indent);
    }
}

} // api
} // storage
