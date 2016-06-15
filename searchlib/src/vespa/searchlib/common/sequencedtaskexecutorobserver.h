// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "isequencedtaskexecutor.h"
#include <atomic>

namespace search
{

/**
 * Observer class to observe class to run multiple tasks in parallel,
 * but tasks with same id has to be run in sequence.
 */
class SequencedTaskExecutorObserver : public ISequencedTaskExecutor
{
    ISequencedTaskExecutor &_executor;
    std::atomic<uint32_t> _executeCnt;
    std::atomic<uint32_t> _syncCnt;
public:
    SequencedTaskExecutorObserver(ISequencedTaskExecutor &executor)
        : _executor(executor),
          _executeCnt(0u),
          _syncCnt(0u)
    {
    }

    virtual ~SequencedTaskExecutorObserver() { }

    virtual void executeTask(uint64_t id,
                             vespalib::Executor::Task::UP task) override {
        ++_executeCnt;
        _executor.executeTask(id, std::move(task));
    }

    virtual void sync() override {
        ++_syncCnt;
        _executor.sync();
    }

    uint32_t getExecuteCnt() const { return _executeCnt; }
    uint32_t getSyncCnt() const { return _syncCnt; }
};

} // namespace search
