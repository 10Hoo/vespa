// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".engine.monitorreply");
#include "monitorreply.h"

namespace search {
namespace engine {

MonitorReply::MonitorReply()
    : mld(),
      activeDocsRequested(false),
      partid(),
      timestamp(),
      totalNodes(),
      activeNodes(),
      totalParts(),
      activeParts(),
      activeDocs(0),
      flags()
{
}

} // namespace engine
} // namespace search
