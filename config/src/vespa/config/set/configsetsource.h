// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/config/common/source.h>
#include <vespa/config/common/configkey.h>
#include <vespa/config/common/iconfigholder.h>
#include <vespa/config/common/configstate.h>
#include <map>

namespace config {

class ConfigInstance;

/**
 * Class for sending and receiving config request from a raw string.
 */
class ConfigSetSource : public Source {
public:
    typedef std::map<ConfigKey, ConfigInstance *> BuilderMap;
    typedef std::shared_ptr<BuilderMap> BuilderMapSP;
    ConfigSetSource(const IConfigHolder::SP & holder, const ConfigKey & key, const BuilderMapSP & builderMap);

    void getConfig();
    void reload(int64_t generation);
    void close();
private:
    IConfigHolder::SP _holder;
    const ConfigKey _key;
    int64_t _generation;
    BuilderMapSP _builderMap;
    int64_t _lastGeneration;
    ConfigState _lastState;

    bool validRequest(const ConfigKey & key);
};

}

