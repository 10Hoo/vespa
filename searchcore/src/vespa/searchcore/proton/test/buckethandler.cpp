// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>

#include "buckethandler.h"


namespace proton
{

namespace test
{


BucketHandler::BucketHandler()
    : IBucketStateChangedNotifier(),
      _handlers()
{
}


BucketHandler::~BucketHandler()
{
    assert(_handlers.empty());
}


void
BucketHandler::addBucketStateChangedHandler(IBucketStateChangedHandler *handler)
{
    _handlers.insert(handler);
}


void
BucketHandler::removeBucketStateChangedHandler(IBucketStateChangedHandler *
                                               handler)
{
    _handlers.erase(handler);
}


void
BucketHandler::notifyBucketStateChanged(const document::BucketId &bucketId,
                                        storage::spi::BucketInfo::ActiveState
                                        newState)
{
    for (auto &handler : _handlers) {
        handler->notifyBucketStateChanged(bucketId, newState);
    }
}


} // namespace test

} // namespace proton

