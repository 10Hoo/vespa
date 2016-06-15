// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "i_job_tracker.h"
#include <vespa/searchcorespi/flush/iflushtarget.h>

namespace proton {

/**
 * Class that tracks the start and end of an init flush in a flush target.
 * The returned flush task is also tracked.
 */
class JobTrackedFlushTarget : public searchcorespi::IFlushTarget
{
private:
    IJobTracker::SP                 _tracker;
    searchcorespi::IFlushTarget::SP _target;

public:
    JobTrackedFlushTarget(const IJobTracker::SP &tracker,
                          const searchcorespi::IFlushTarget::SP &target);

    const IJobTracker &getTracker() const { return *_tracker; }
    const searchcorespi::IFlushTarget &getTarget() const { return *_target; }

    // Implements searchcorespi::IFlushTarget
    virtual MemoryGain getApproxMemoryGain() const {
        return _target->getApproxMemoryGain();
    }
    virtual DiskGain getApproxDiskGain() const {
        return _target->getApproxDiskGain();
    }
    virtual SerialNum getFlushedSerialNum() const {
        return _target->getFlushedSerialNum();
    }
    virtual Time getLastFlushTime() const {
        return _target->getLastFlushTime();
    }
    virtual bool needUrgentFlush() const {
        return _target->needUrgentFlush();
    }
    virtual searchcorespi::FlushTask::UP initFlush(SerialNum currentSerial);
    virtual searchcorespi::FlushStats getLastFlushStats() const {
        return _target->getLastFlushStats();
    }

    virtual uint64_t getApproxBytesToWriteToDisk() const override;
};

} // namespace proton

