// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "configformatter.h"

namespace config {

/**
 * Formatter capable of encoding and decoding config as json.
 */
class JsonConfigFormatter : public ConfigFormatter {
public:
    JsonConfigFormatter(bool compact = false);
    // Inherits ConfigFormatter
    void encode(ConfigDataBuffer & buffer) const;
    // Inherits ConfigFormatter
    size_t decode(ConfigDataBuffer & buffer) const;
private:
    const bool _compact;
};

} // namespace config

