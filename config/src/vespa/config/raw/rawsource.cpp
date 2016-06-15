// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/config/common/misc.h>
#include "rawsource.h"
#include <vespa/vespalib/stllike/asciistream.h>

namespace config {


RawSource::RawSource(const IConfigHolder::SP & holder, const vespalib::string & payload)
    : _holder(holder),
      _payload(payload)
{
}

void
RawSource::getConfig()
{
    auto lines(readConfig());
    ConfigValue value(lines, calculateContentMd5(lines));
    _holder->handle(ConfigUpdate::UP(new ConfigUpdate(value, true, 1)));
}

void
RawSource::reload(int64_t generation)
{
    (void) generation;
}

void
RawSource::close()
{
}

std::vector<vespalib::string>
RawSource::readConfig()
{
    vespalib::asciistream is(_payload);
    return is.getlines();
}

}
