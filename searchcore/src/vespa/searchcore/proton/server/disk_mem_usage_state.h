// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "resource_usage_state.h"

namespace proton {

/**
 * Class used to describe state of disk and memory usage relative to configured limits.
 */
class DiskMemUsageState
{
    ResourceUsageState _diskState;
    ResourceUsageState _memoryState;

public:
    DiskMemUsageState()
        : _diskState(),
          _memoryState()
    {
    }
    DiskMemUsageState(const ResourceUsageState &diskState_,
                      const ResourceUsageState &memoryState_)
        : _diskState(diskState_),
          _memoryState(memoryState_)
    {
    }
    bool operator==(const DiskMemUsageState &rhs) const {
        return ((_diskState == rhs._diskState) &&
                (_memoryState == rhs._memoryState));
    }
    bool operator!=(const DiskMemUsageState &rhs) const {
        return ! ((*this) == rhs);
    }
    const ResourceUsageState &diskState() const { return _diskState; }
    const ResourceUsageState &memoryState() const { return _memoryState; }
    bool aboveDiskLimit() const { return diskState().aboveLimit(); }
    bool aboveMemoryLimit() const { return memoryState().aboveLimit(); }
};

} // namespace proton
