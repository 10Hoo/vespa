// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/storage/distributor/throttlingoperationstarter.h>

namespace storage {
namespace distributor {

ThrottlingOperationStarter::ThrottlingOperation::~ThrottlingOperation()
{
    _operationStarter.signalOperationFinished(*this);
}

bool
ThrottlingOperationStarter::canStart(uint32_t currentOperationCount,
                                    Priority priority) const
{
    uint32_t variablePending(_maxPending - _minPending);
    uint32_t maxPendingForPri(_minPending + variablePending*((255.0 - priority) / 255.0));

    return currentOperationCount < maxPendingForPri;
}

bool
ThrottlingOperationStarter::start(const std::shared_ptr<Operation>& operation,
                                 Priority priority)
{
    if (!canStart(_pendingCount, priority)) {
        return false;
    }
    Operation::SP wrappedOp(new ThrottlingOperation(operation, *this));
    ++_pendingCount;
    return _starterImpl.start(wrappedOp, priority);
}

void
ThrottlingOperationStarter::signalOperationFinished(const Operation& op)
{
    (void) op;
    assert(_pendingCount > 0);
    --_pendingCount;
}

}
}
