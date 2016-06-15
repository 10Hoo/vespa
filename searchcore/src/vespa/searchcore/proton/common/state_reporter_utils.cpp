// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".proton.common.state_reporter_utils");

#include "state_reporter_utils.h"
#include <vespa/vespalib/data/slime/cursor.h>

using namespace vespalib::slime;

namespace proton {

void
StateReporterUtils::convertToSlime(const StatusReport &statusReport,
                                   const Inserter &inserter)
{
    Cursor &object = inserter.insertObject();
    object.setString("state", statusReport.getInternalState());
    if (statusReport.hasProgress()) {
        object.setDouble("progress", statusReport.getProgress());
    }
    if (!statusReport.getInternalConfigState().empty()) {
        object.setString("configState", statusReport.getInternalConfigState());
    }
    if (!statusReport.getMessage().empty()) {
        object.setString("message", statusReport.getMessage());
    }
}

} // namespace proton

