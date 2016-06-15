// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/vespalib/stllike/asciistream.h>
#include "configsnapshotreader.h"

namespace config {

/**
 * Read config snapshots from an ascii stream.
 */
class AsciiConfigSnapshotReader : public ConfigSnapshotReader {
public:
    AsciiConfigSnapshotReader(const vespalib::asciistream & is);

    /**
     * Read a config snapshot.
     *
     * @return Snapshot containing the configs.
     */
    ConfigSnapshot read();
private:
    const vespalib::asciistream & _is;
};

} // namespace config

