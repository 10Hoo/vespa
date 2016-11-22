// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "bucketspace.h"
#include "bucketid.h"
#include <vespa/vespalib/stllike/string.h>
#include <vespa/vespalib/stllike/asciistream.h>
#include <cstdint>
#include <iostream>

namespace document {

class Bucket {
public:
    Bucket(const Bucket&) noexcept = default;
    Bucket& operator=(const Bucket&) noexcept = default;
    Bucket(BucketSpace bucketSpace, BucketId bucketId) noexcept : _bucketSpace(bucketSpace), _bucketId(bucketId) {}

    bool operator<(const Bucket& other) const noexcept {
        if (_bucketSpace == other._bucketSpace) {
            return _bucketId < other._bucketId;
        }
        return _bucketSpace < other._bucketSpace;
    }
    bool operator==(const Bucket& other) const noexcept {
        return _bucketSpace == other._bucketSpace && _bucketId == other._bucketId;
    }
    bool operator!=(const Bucket& other) const noexcept { return !(operator==(other)); }

    BucketSpace getBucketSpace() const noexcept { return _bucketSpace; }
    BucketId getBucketId() const noexcept { return _bucketId; }
    void print(std::ostream& os) const;
    vespalib::string toString() const;

    struct hash {
        size_t operator () (const Bucket& b) const {
            size_t hash1 = BucketId::hash()(b.getBucketId());
            size_t hash2 = BucketSpace::hash()(b.getBucketSpace());
            // Formula taken from std::hash_combine proposal
            return hash1 ^ (hash2 + 0x9e3779b9 + (hash1<<6) + (hash1>>2));
        }
    };
private:
    BucketSpace _bucketSpace;
    BucketId _bucketId;
};

std::ostream& operator<<(std::ostream&, const Bucket&);
vespalib::asciistream& operator<<(vespalib::asciistream&, const Bucket&);

}