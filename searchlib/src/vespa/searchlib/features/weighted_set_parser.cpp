// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".features.weighted_set_parser");

#include "weighted_set_parser.h"

namespace search {
namespace features {

void
WeightedSetParser::logWarning(const vespalib::string &msg)
{
    LOG(warning, "%s", msg.c_str());
}

} // namespace features
} // namespace search
