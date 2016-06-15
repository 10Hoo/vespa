// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/docstore/idocumentstore.h>
#include <vespa/searchcorespi/flush/iflushtarget.h>

namespace proton {

using searchcorespi::FlushStats;
using searchcorespi::IFlushTarget;

/**
 * This class implements the IFlushTarget interface to proxy a summary manager.
 */
class SummaryFlushTarget : public IFlushTarget {
private:
    search::IDocumentStore & _docStore;
    FlushStats _lastStats;

public:
    SummaryFlushTarget(search::IDocumentStore & docStore);

    // Implements IFlushTarget
    virtual MemoryGain getApproxMemoryGain() const;
    virtual   DiskGain getApproxDiskGain() const;
    virtual  SerialNum getFlushedSerialNum() const;
    virtual       Time getLastFlushTime() const;

    virtual Task::UP initFlush(SerialNum currentSerial);

    virtual FlushStats getLastFlushStats() const { return _lastStats; }
    virtual uint64_t getApproxBytesToWriteToDisk() const override;
};

} // namespace proton

