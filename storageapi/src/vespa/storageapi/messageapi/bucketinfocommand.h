// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * @class storage::api::BucketInfoCommand
 * @ingroup messageapi
 *
 * @brief Superclass for storage commands that returns bucket info.
 *
 * This class doesn't add any functionality now, other than being able to check
 * if a message is an instance of this class. But we want commands and replies
 * to be in the same inheritance structure, and the reply adds functionality.
 */

#pragma once

#include <vespa/storageapi/messageapi/bucketcommand.h>

namespace storage {
namespace api {

class BucketInfoCommand : public BucketCommand {
protected:
    BucketInfoCommand(const MessageType& type, const document::BucketId& id)
        : BucketCommand(type, id) {}

public:
    DECLARE_POINTER_TYPEDEFS(BucketInfoCommand);

    virtual void print(std::ostream& out, bool verbose,
                       const std::string& indent) const;
};

} // api
} // storage

