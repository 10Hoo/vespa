// Copyright 2017 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchcorespi/flush/iflushtarget.h>

namespace search::common { class ICompactableLidSpace; }

namespace proton {


/**
 * Implements a flush target that shrinks lid space in target.
 */
class ShrinkLidSpaceFlushTarget : public searchcorespi::IFlushTarget
{
    /**
     * Task representing that shrinking has been performed.
     **/
    class Flusher;
    using ICompactableLidSpace = search::common::ICompactableLidSpace;
    using FlushStats = searchcorespi::FlushStats;
    std::shared_ptr<ICompactableLidSpace> _target;
    SerialNum                             _flushedSerialNum;
    FlushStats                            _lastStats;

public:
    /**
     * Constructs a new instance of this class.
     *
     * @param name                The handler-wide unique name of this target.
     * @param type                The flush type of this target.
     * @param component           The component type of this target.
     * @param flushedSerialNum    When target shrank lid space last time
     * @param target              The target supporting lid space compaction
     */
    ShrinkLidSpaceFlushTarget(const vespalib::string &name,
                              Type type,
                              Component component,
                              SerialNum flushedSerialNum,
                              std::shared_ptr<ICompactableLidSpace> target);

    // Implements IFlushTarget.
    virtual MemoryGain getApproxMemoryGain() const override;
    virtual DiskGain getApproxDiskGain() const override;
    virtual SerialNum getFlushedSerialNum() const override;
    virtual Time getLastFlushTime() const override;
    virtual bool needUrgentFlush() const override;
    virtual Task::UP initFlush(SerialNum currentSerial) override;
    virtual searchcorespi::FlushStats getLastFlushStats() const override;
    virtual uint64_t getApproxBytesToWriteToDisk() const override;
};

} // namespace proton
