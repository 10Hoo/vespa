// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "feedtoken.h"
#include <vespa/searchcore/proton/metrics/feed_metrics.h>

namespace proton {

FeedToken::FeedToken(ITransport &transport, mbus::Reply::UP reply) :
    _state(new State(transport, std::move(reply), 1))
{
}

FeedToken::State::State(ITransport & transport, mbus::Reply::UP reply, uint32_t numAcksRequired) :
    _transport(transport),
    _reply(std::move(reply)),
    _result(new storage::spi::Result()),
    _documentWasFound(false),
    _unAckedCount(numAcksRequired),
    _lock(),
    _startTime()
{
    assert(_reply);
    _startTime.SetNow();
}

FeedToken::State::~State()
{
    assert(!_reply);
}

void
FeedToken::State::ack()
{
    assert(_reply);
    uint32_t prev(_unAckedCount--);
    if (prev == 1) {
        _transport.send(std::move(_reply), std::move(_result), _documentWasFound, _startTime.MilliSecsToNow());
    }
    assert(prev >= 1);
}


void
FeedToken::State::ack(const FeedOperation::Type opType,
                      PerDocTypeFeedMetrics &metrics)
{
    assert(_reply);
    uint32_t prev(_unAckedCount--);
    if (prev == 1) {
        _transport.send(std::move(_reply), std::move(_result), _documentWasFound, _startTime.MilliSecsToNow());
        switch (opType) {
        case FeedOperation::PUT:
            metrics.RegisterPut(_startTime);
            break;
        case FeedOperation::REMOVE:
        case FeedOperation::REMOVE_BATCH:
            metrics.RegisterRemove(_startTime);
            break;
        case FeedOperation::UPDATE_42:
        case FeedOperation::UPDATE:
            metrics.RegisterUpdate(_startTime);
            break;
        case FeedOperation::MOVE:
            metrics.RegisterMove(_startTime);
            break;
        default:
            ;
        }
    }
    assert(prev >= 1);
}


void
FeedToken::State::incNeededAcks()
{
    assert(_reply);
    uint32_t prev(_unAckedCount++);
    assert(prev >= 1);
    (void) prev;
}


void
FeedToken::State::fail(uint32_t errNum, const vespalib::string &errMsg)
{
    assert(_reply);
    vespalib::LockGuard guard(_lock);
    _reply->addError(mbus::Error(errNum, errMsg));
    _transport.send(std::move(_reply), std::move(_result), _documentWasFound, _startTime.MilliSecsToNow());
}

} // namespace proton
