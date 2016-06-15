// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "configwriter.h"
#include "configformatter.h"
#include <vespa/vespalib/stllike/asciistream.h>

namespace config {

class AsciiConfigWriter : public ConfigWriter {
public:
    AsciiConfigWriter(vespalib::asciistream & os);
    bool write(const ConfigInstance & config);
    bool write(const ConfigInstance & config, const ConfigFormatter & formatter);
private:
    vespalib::asciistream & _os;
};

} // namespace config

