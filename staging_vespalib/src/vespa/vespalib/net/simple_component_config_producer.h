// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "component_config_producer.h"
#include <vespa/vespalib/util/sync.h>
#include <map>

namespace vespalib {

class SimpleComponentConfigProducer : public ComponentConfigProducer
{
private:
    Lock _lock;
    std::map<vespalib::string, Config> _state;

public:
    SimpleComponentConfigProducer();
    void addConfig(const Config &config);
    void removeConfig(const vespalib::string &name);
    virtual void getComponentConfig(Consumer &consumer);
};

} // namespace vespalib

