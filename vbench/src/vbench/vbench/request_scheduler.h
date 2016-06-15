// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.


#pragma once

#include <memory>

#include <vespa/vespalib/util/sync.h>
#include <vespa/vespalib/util/thread.h>
#include <vespa/vespalib/util/runnable.h>
#include <vespa/vespalib/util/active.h>

#include <vbench/core/handler.h>
#include <vbench/core/time_queue.h>
#include <vbench/core/dispatcher.h>
#include <vbench/core/handler_thread.h>

#include "request.h"
#include "worker.h"
#include "dropped_tagger.h"

namespace vbench {

/**
 * Component responsible for dispatching requests to workers at the
 * appropriate time based on what start time the requests are tagged
 * with.
 **/
class RequestScheduler : public Handler<Request>,
                         public vespalib::Runnable,
                         public vespalib::Active
{
private:
    Timer                   _timer;
    HandlerThread<Request>  _proxy;
    TimeQueue<Request>      _queue;
    DroppedTagger           _droppedTagger;
    Dispatcher<Request>     _dispatcher;
    vespalib::Thread        _thread;
    HttpConnectionPool      _connectionPool;
    std::vector<Worker::UP> _workers;

    virtual void run();
public:
    typedef std::unique_ptr<RequestScheduler> UP;
    RequestScheduler(Handler<Request> &next, size_t numWorkers);
    void abort();
    virtual void handle(Request::UP request);
    virtual void start();
    virtual RequestScheduler &stop();
    virtual void join();
};

} // namespace vbench

