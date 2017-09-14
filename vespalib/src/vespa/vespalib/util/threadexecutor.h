// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Copyright (C) 2010 Yahoo

#pragma once

#include <vespa/vespalib/util/executor.h>
#include <vespa/vespalib/util/syncable.h>

namespace vespalib {

/**
 * Can both execute and sync
 **/
class ThreadExecutor : public Executor,
                       public Syncable
{
public:
    /**
     * Get number of threads in the executor pool.
     * @return number of threads in the pool
     */
    virtual size_t getNumThreads() const = 0;
};

} // namespace vespalib

