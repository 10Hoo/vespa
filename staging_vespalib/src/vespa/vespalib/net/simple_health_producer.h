// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "health_producer.h"
#include <vespa/vespalib/util/sync.h>

namespace vespalib {

class SimpleHealthProducer : public HealthProducer
{
private:
    Lock _lock;
    HealthProducer::Health _health;

public:
    SimpleHealthProducer();
    void setOk();
    void setFailed(const vespalib::string &msg);
    virtual Health getHealth() const;
};

} // namespace vespalib

