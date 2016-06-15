// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * @class storage::api::BucketCommand
 * @ingroup messageapi
 *
 * @brief Superclass for storage commands that operate towards a single bucket.
 */

#pragma once

#include <vespa/document/bucket/bucketid.h>
#include <vespa/storageapi/messageapi/storagecommand.h>

namespace storage {
namespace api {

class BucketCommand : public StorageCommand {
    document::BucketId _bucket;
    document::BucketId _originalBucket;

protected:
    BucketCommand(const MessageType& type, const document::BucketId& id)
        : StorageCommand(type), _bucket(id), _originalBucket() {}

public:
    DECLARE_POINTER_TYPEDEFS(BucketCommand);

    void remapBucketId(const document::BucketId& bucket) {
        if (_originalBucket.getRawId() == 0) _originalBucket = _bucket;
        _bucket = bucket;
    }

    document::BucketId getBucketId() const { return _bucket; }
    bool hasBeenRemapped() const { return (_originalBucket.getRawId() != 0); }
    const document::BucketId& getOriginalBucketId() const
        { return _originalBucket; }

    virtual void print(std::ostream& out,
                       bool verbose, const std::string& indent) const;

    virtual bool hasSingleBucketId() const { return true; }

};

} // api
} // storage

