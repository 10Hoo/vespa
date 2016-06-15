// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/config/common/sourcefactory.h>
#include "configsetsource.h"

namespace config {

class Source;
class IConfigHolder;
class ConfigKey;

/**
 * Factory creating config payload from config instances.
 */
class ConfigSetSourceFactory : public SourceFactory
{
public:
    typedef ConfigSetSource::BuilderMap BuilderMap;
    typedef ConfigSetSource::BuilderMapSP BuilderMapSP;
    ConfigSetSourceFactory(const BuilderMapSP & builderMap);

    /**
     * Create source handling config described by key.
     */
    Source::UP createSource(const IConfigHolder::SP & holder, const ConfigKey & key) const;
private:
    BuilderMapSP _builderMap;
};

} // namespace config

