// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include "versionreporter.h"
#include <vespa/storage/common/vtag.h>
#include <vespa/vespalib/component/version.h>

namespace storage {

namespace {
using Object = vespalib::JsonStream::Object;
using End = vespalib::JsonStream::End;
}
void VersionReporter::report(vespalib::JsonStream& jsonreport) {
    jsonreport << "vtag" << Object()
               << "version" << Vtag::currentVersion.toString()
               << End();
}

} /* namespace storage */
