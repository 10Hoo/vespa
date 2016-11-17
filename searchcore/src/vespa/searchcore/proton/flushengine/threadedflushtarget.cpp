// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".proton.flushengine.threadedflushtarget");

#include "threadedflushtarget.h"
#include <vespa/vespalib/util/executor.h>
#include <vespa/searchcore/proton/server/igetserialnum.h>
#include <vespa/searchlib/common/lambdatask.h>
#include <future>

using searchcorespi::IFlushTarget;
using searchcorespi::FlushStats;
using search::makeLambdaTask;

namespace proton {

ThreadedFlushTarget::ThreadedFlushTarget(vespalib::Executor &executor,
                                         const IGetSerialNum &getSerialNum,
                                         const IFlushTarget::SP &target)
    : FlushTargetProxy(target),
      _executor(executor),
      _getSerialNum(getSerialNum)
{
}

ThreadedFlushTarget::ThreadedFlushTarget(vespalib::Executor &executor,
                                         const IGetSerialNum &getSerialNum,
                                         const IFlushTarget::SP &target,
                                         const vespalib::string & prefix)
    : FlushTargetProxy(target, prefix),
      _executor(executor),
      _getSerialNum(getSerialNum)
{
}

namespace {
IFlushTarget::Task::UP
callInitFlush(IFlushTarget *target, IFlushTarget::SerialNum serial,
              const IGetSerialNum *getSerialNum) {
    // Serial number from flush engine might have become stale, obtain
    // a fresh serial number now.
    (void) serial;
    search::SerialNum freshSerial = getSerialNum->getSerialNum();
    assert(freshSerial >= serial);
    return target->initFlush(freshSerial);
}
}  // namespace

IFlushTarget::Task::UP
ThreadedFlushTarget::initFlush(SerialNum currentSerial)
{
    std::promise<Task::UP> promise;
    std::future<Task::UP> future = promise.get_future();
    _executor.execute(makeLambdaTask([&]()
                      { promise.set_value(callInitFlush(_target.get(),
                                                        currentSerial,
                                                        &_getSerialNum)); }));
    return future.get();
}

} // namespace proton
