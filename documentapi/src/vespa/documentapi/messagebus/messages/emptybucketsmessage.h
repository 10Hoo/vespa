// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/document/bucket/bucketid.h>
#include <vespa/documentapi/messagebus/messages/visitor.h>

namespace documentapi {

/**
 * @class EmptyBucketsMessage
 * @ingroup message
 *
 * @brief Encapsulates a set of Empty bucket ids.
 */
class EmptyBucketsMessage : public VisitorMessage {
private:
    std::vector<document::BucketId> _bucketIds;

protected:
    DocumentReply::UP doCreateReply() const;

public:
    EmptyBucketsMessage(); // must be serialized into

    EmptyBucketsMessage(const std::vector<document::BucketId> &bucketIds);

    std::vector<document::BucketId> &getBucketIds() { return _bucketIds; }
    const std::vector<document::BucketId> &getBucketIds() const { return _bucketIds; }

    void setBucketIds(const std::vector<document::BucketId> &bucketIds);

    void resize(uint32_t size);

    uint32_t getType() const;

    string toString() const { return "emptybucketsmessage"; }
};

}

