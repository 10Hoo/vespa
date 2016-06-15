// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/vespalib/stllike/string.h>
#include "configsnapshotreader.h"

namespace config {

/**
 * Read config snapshots from file.
 */
class FileConfigSnapshotReader : public ConfigSnapshotReader {
public:
    FileConfigSnapshotReader(const vespalib::string & fileName);

    /**
     * Read a config snapshot.
     *
     * @return Snapshot containing the configs.
     */
    ConfigSnapshot read();
private:
    const vespalib::string _fileName;
};

} // namespace config

