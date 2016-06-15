// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * @class storage::api::BucketReply
 * @ingroup messageapi
 *
 * @brief Superclass for storage replies which operates on single bucket.
 */

#pragma once

#include <vespa/document/bucket/bucketid.h>
#include <vespa/storageapi/messageapi/storagereply.h>

namespace storage {
namespace api {

class BucketCommand;

class BucketReply : public StorageReply {
    document::BucketId _bucket;
    document::BucketId _originalBucket;

protected:
    BucketReply(const BucketCommand& cmd,
                const ReturnCode& code = ReturnCode(ReturnCode::OK));

public:
    DECLARE_POINTER_TYPEDEFS(BucketReply);

    document::BucketId getBucketId() const { return _bucket; }
    virtual bool hasSingleBucketId() const { return true; }

    bool hasBeenRemapped() const { return (_originalBucket.getRawId() != 0); }
    const document::BucketId& getOriginalBucketId() const
        { return _originalBucket; }

    /** The deserialization code need access to set the remapping. */
    void remapBucketId(const document::BucketId& bucket) {
        if (_originalBucket.getRawId() == 0) _originalBucket = _bucket;
        _bucket = bucket;
    }

    virtual void print(std::ostream& out, bool verbose,
                       const std::string& indent) const;
};

} // api
} // storage

