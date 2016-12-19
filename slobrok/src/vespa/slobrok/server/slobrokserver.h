// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "sbenv.h"
#include "configshim.h"
#include <vespa/vespalib/util/thread.h>
#include <vespa/vespalib/util/runnable.h>
#include <vespa/config/config.h>
#include <vespa/config-slobroks.h>

namespace slobrok {


class SlobrokServer : public vespalib::Runnable
{
private:
    SBEnv              _env;
    vespalib::Thread   _thread;

public:
    SlobrokServer(ConfigShim &shim);
    SlobrokServer(uint32_t port);
    ~SlobrokServer();

    virtual void run();

    void stop() { _env.shutdown(); }
};

} // namespace slobrok

