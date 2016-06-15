// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/document/base/globalid.h>
#include <vespa/document/bucket/bucketid.h>
#include <vespa/persistence/spi/bucketinfo.h>
#include <vespa/searchcore/proton/common/subdbtype.h>
#include <vespa/persistence/spi/result.h>
#include "bucketstate.h"

namespace proton
{

class BucketDB
{
public:
    typedef document::GlobalId GlobalId;
    typedef document::BucketId BucketId;
    typedef storage::spi::Timestamp Timestamp;
    typedef storage::spi::BucketChecksum BucketChecksum;
    typedef bucketdb::BucketState BucketState;
    typedef std::map<BucketId, BucketState> Map;
    typedef Map::iterator MapIterator;
    typedef Map::const_iterator ConstMapIterator;
    typedef std::pair<MapIterator, bool> InsertResult;

private:
    Map _map;
    BucketId _cachedBucketId;
    BucketState _cachedBucketState;

    void clear(void);
    void checkEmpty(void) const;
public:
    BucketDB(void);
    virtual ~BucketDB(void);

    const BucketState &
    add(const GlobalId &gid,
        const BucketId &bucketId,
        const Timestamp &timestamp,
        SubDbType subDbType);

    void add(const BucketId &bucketId, const BucketState & state) {
        _map[bucketId] += state;
    }

    void
    remove(const GlobalId &gid,
           const BucketId &bucketId,
           const Timestamp &timestamp,
           SubDbType subDbType);

    void
    modify(const GlobalId &gid,
           const BucketId &oldBucketId,
           const Timestamp &oldTimestamp,
           const BucketId &newBucketId,
           const Timestamp &newTimestamp,
           SubDbType subDbType);

    BucketState get(const BucketId &bucketId) const;
    void cacheBucket(const BucketId &bucketId);
    void uncacheBucket();
    bool isCachedBucket(const BucketId &bucketId) const;
    BucketState cachedGet(const BucketId &bucketId) const;
    bool hasBucket(const BucketId &bucketId) const;
    void getBuckets(BucketId::List & buckets) const;
    bool empty(void) const;
    void setBucketState(const BucketId &bucketId, bool active);
    void createBucket(const BucketId &bucketId);
    void deleteEmptyBucket(const BucketId &bucketId);
    void getActiveBuckets(BucketId::List &buckets) const;
    void populateActiveBuckets(const BucketId::List &buckets, BucketId::List &fixupBuckets);

    ConstMapIterator begin() const { return _map.begin(); }
    ConstMapIterator end() const { return _map.end(); }
    ConstMapIterator lowerBound(const BucketId &bucket) const { return _map.lower_bound(bucket); }
    ConstMapIterator upperBound(const BucketId &bucket) const { return _map.upper_bound(bucket); }
    size_t size() const { return _map.size(); }
    bool isActiveBucket(const BucketId &bucketId) const;
    BucketState *getBucketStatePtr(const BucketId &bucket);
    void unloadBucket(const BucketId &bucket, const BucketState &delta);
};

}

