// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/storageapi/messageapi/bucketinfocommand.h>

#include <vespa/storageapi/message/persistence.h>

namespace storage {
namespace api {

void
BucketInfoCommand::print(std::ostream& out, bool verbose,
                         const std::string& indent) const
{
    out << "BucketInfoCommand()";
    if (verbose) {
        out << " : ";
        BucketCommand::print(out, verbose, indent);
    }
}

} // api
} // storage
