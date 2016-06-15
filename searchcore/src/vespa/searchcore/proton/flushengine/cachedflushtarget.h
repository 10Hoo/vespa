// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchcorespi/flush/iflushtarget.h>

namespace proton {

using searchcorespi::FlushStats;
using searchcorespi::IFlushTarget;

/**
 * Implements a flush target that caches the flushable memory and flush cost of
 * a decorated target. This is used by the flush engine to avoid recalculating
 * these during selection of flush target.
 */
class CachedFlushTarget : public IFlushTarget {
private:
    IFlushTarget::SP  _target;
    SerialNum         _flushedSerialNum;
    Time              _lastFlushTime;
    MemoryGain        _memoryGain;
    DiskGain          _diskGain;
    bool              _needUrgentFlush;
    uint64_t          _approxBytesToWriteToDisk;

public:
    /**
     * Constructs a new instance of this class. This will immediately call
     * getFlushableMemory(), getFlushCost() and getLowSerialNum() on the
     * argument target.
     *
     * @param target The target to decorate.
     */
    CachedFlushTarget(const IFlushTarget::SP &target);

    /**
     * Returns the decorated flush target. This should not be used for anything
     * but testing, as invoking a method on the returned target beats the
     * purpose of decorating it.
     *
     * @return The decorated flush target.
     */
    const IFlushTarget::SP & getFlushTarget() { return _target; }

    // Implements IFlushTarget.
    virtual MemoryGain getApproxMemoryGain() const { return _memoryGain; }
    virtual   DiskGain   getApproxDiskGain() const { return _diskGain; }
    virtual  SerialNum getFlushedSerialNum() const { return _flushedSerialNum; }
    virtual       Time    getLastFlushTime() const { return _lastFlushTime; }
    virtual       bool     needUrgentFlush() const { return _needUrgentFlush; }

    virtual Task::UP initFlush(SerialNum currentSerial) { return _target->initFlush(currentSerial); }
    virtual FlushStats getLastFlushStats() const { return _target->getLastFlushStats(); }

    virtual uint64_t getApproxBytesToWriteToDisk() const override;
};

} // namespace proton

