// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "i_disk_mem_usage_listener.h"
#include "memoryflush.h"
#include <vespa/searchcore/config/config-proton.h>
#include <mutex>

namespace proton {

/**
 * Class that listens to changes in disk and memory usage and
 * updates the config used by memory flush strategy accordingly if we reach one of the resource limits.
 */
class MemoryFlushConfigUpdater : public IDiskMemUsageListener
{
private:
    using Mutex = std::mutex;
    using LockGuard = std::lock_guard<Mutex>;
    using ProtonConfig = vespa::config::search::core::ProtonConfig;

    MemoryFlush::SP _flushStrategy;
    ProtonConfig::Flush::Memory _currConfig;
    DiskMemUsageState _currState;
    Mutex _mutex;

    void updateFlushStrategy(const LockGuard &);

public:
    using UP = std::unique_ptr<MemoryFlushConfigUpdater>;

    MemoryFlushConfigUpdater(const MemoryFlush::SP &flushStrategy,
                             const ProtonConfig::Flush::Memory &config);
    void setConfig(const ProtonConfig::Flush::Memory &newConfig);
    virtual void notifyDiskMemUsage(DiskMemUsageState newState) override;

    static MemoryFlush::Config convertConfig(const ProtonConfig::Flush::Memory &config);
};

} // namespace proton
