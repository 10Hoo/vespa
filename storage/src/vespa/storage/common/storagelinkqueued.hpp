// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "storagelinkqueued.h"
#include <vespa/vespalib/util/stringfmt.h>
#include <sstream>

namespace storage {

template<typename Message>
void
StorageLinkQueued::Dispatcher<Message>::terminate() {
    if (_thread.get()) {
        _thread->interrupt();
        {
            vespalib::MonitorGuard sync(_sync);
            sync.signal();
        }
        _thread->join();
        _thread.reset(0);
    }
}

template<typename Message>
StorageLinkQueued::Dispatcher<Message>::Dispatcher(StorageLinkQueued& parent, unsigned int maxQueueSize, bool replyDispatcher)
    : _parent(parent),
      _maxQueueSize(maxQueueSize),
      _sync(),
      _messages(),
      _replyDispatcher(replyDispatcher)
{
    std::ostringstream name;
    name << "Queued storage " << (_replyDispatcher ? "up" : "down")
         << "link - " << _parent.getName();
    _component.reset(new framework::Component(
            parent.getComponentRegister(),
            name.str()));
}

template<typename Message>
StorageLinkQueued::Dispatcher<Message>::~Dispatcher() {
    terminate();
}

template<typename Message>
void StorageLinkQueued::Dispatcher<Message>::start()
{
    assert(_thread.get() == 0);
    framework::MilliSecTime maxProcessTime(5 * 1000);
    framework::MilliSecTime waitTime(100);
    _thread = _component->startThread(*this, maxProcessTime, waitTime);
}

template<typename Message>
void StorageLinkQueued::Dispatcher<Message>::add(
        const std::shared_ptr<Message>& m)
{
    vespalib::MonitorGuard sync(_sync);

    if (_thread.get() == 0) start();
    while ((_messages.size() > _maxQueueSize) && !_thread->interrupted()) {
        sync.wait(100);
    }
    _messages.push_back(m);
    sync.signal();
}

template<typename Message>
void StorageLinkQueued::Dispatcher<Message>::addWithoutLocking(
        const std::shared_ptr<Message>& m)
{
    if (_thread.get() == 0) start();
    _messages.push_back(m);
}

template<typename Message>
void StorageLinkQueued::Dispatcher<Message>::run(framework::ThreadHandle& h)
{
    while (!h.interrupted()) {
        h.registerTick(framework::PROCESS_CYCLE);
        std::shared_ptr<Message> message;
        {
            vespalib::MonitorGuard sync(_sync);
            while (!h.interrupted() && _messages.empty()) {
                sync.wait(100);
                h.registerTick(framework::WAIT_CYCLE);
            }
            if (h.interrupted()) break;
            message.swap(_messages.front());
        }
        try {
            send(message);
        } catch (std::exception& e) {
            _parent.logError(vespalib::make_string(
                    "When running command %s, caught exception %s. "
                    "Discarding message",
                    message->toString().c_str(),
                    e.what()).c_str());
        }

        {
            // Since flush() only waits for stack to be empty, we must
            // pop stack AFTER send have been called.
            vespalib::MonitorGuard sync(_sync);
            _messages.pop_front();
            sync.signal();
        }
    }
    _parent.logDebug("Finished storage link queued thread");
}

template<typename Message>
void StorageLinkQueued::Dispatcher<Message>::flush()
{
    vespalib::MonitorGuard sync(_sync);
    while (!_messages.empty()) {
        sync.wait(100);
    }
}

}

