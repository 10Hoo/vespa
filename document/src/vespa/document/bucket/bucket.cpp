// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "bucket.h"

namespace document {

vespalib::string Bucket::toString() const
{
    vespalib::asciistream os;
    os << *this;
    return os.str();
}

vespalib::asciistream& operator<<(vespalib::asciistream& os, const Bucket& id)
{
    return os << "Bucket(" << id.getBucketSpace() << ", " << id.getBucketId() << ")";
}

}
