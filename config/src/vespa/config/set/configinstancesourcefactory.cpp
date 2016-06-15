// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "configinstancesourcefactory.h"
#include <vespa/config/common/source.h>
#include <vespa/config/common/misc.h>

namespace {

class ConfigInstanceSource : public config::Source {
public:
    ConfigInstanceSource(const config::IConfigHolder::SP & holder, const vespalib::asciistream & buffer)
        : _holder(holder),
          _buffer(buffer),
          _generation(-1)
    { }
    virtual void close() { }
    virtual void getConfig() {
        std::vector<vespalib::string> lines(_buffer.getlines());
        std::string currentMd5(config::calculateContentMd5(lines));
        _holder->handle(config::ConfigUpdate::UP(new config::ConfigUpdate(config::ConfigValue(lines, currentMd5), true, _generation)));

    }
    virtual void reload(int64_t generation) { _generation = generation; }
private:
    config::IConfigHolder::SP _holder;
    vespalib::asciistream _buffer;
    int64_t _generation;
};

}

namespace config {

ConfigInstanceSourceFactory::ConfigInstanceSourceFactory(const ConfigKey & key, const vespalib::asciistream & buffer)
    : _key(key),
      _buffer(buffer)
{
}

Source::UP
ConfigInstanceSourceFactory::createSource(const IConfigHolder::SP & holder, const ConfigKey & key) const
{
    (void) key;
    // TODO: Check key against _key
    return Source::UP(new ConfigInstanceSource(holder, _buffer));
}

} // namespace config

