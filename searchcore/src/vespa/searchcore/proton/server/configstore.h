// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "documentdbconfig.h"
#include "feedconfigstore.h"
#include <vespa/searchcommon/common/schema.h>
#include <vespa/searchcore/config/config-proton.h>

namespace proton {

struct ConfigStore : FeedConfigStore {
    typedef std::unique_ptr<ConfigStore> UP;
    typedef search::SerialNum SerialNum;
    using ProtonConfig = vespa::config::search::core::ProtonConfig;
    using ProtonConfigSP = std::shared_ptr<ProtonConfig>;

    /**
     * @param currentSnapshot the current snapshot, for reusing
     *                        unchanged parts .
     * @param serialNum the serial number of the config snapshot to load.
     * @param loadedSnapshot the shared pointer in which to store the
     *                       resulting config snapshot.
     * @param historySchema the shared pointer in which to store the
     *                      resulting history schema.
     */
    virtual void loadConfig(const DocumentDBConfig &currentSnapshot,
                            SerialNum serialNum,
                            DocumentDBConfig::SP &loadedSnapshot,
                            search::index::Schema::SP &historySchema) = 0;
    virtual void saveConfig(const DocumentDBConfig &snapshot,
                            const search::index::Schema &historySchema,
                            SerialNum serialNum) = 0;

    virtual void removeInvalid() = 0;
    virtual void prune(SerialNum serialNum) = 0;

    virtual SerialNum getBestSerialNum() const = 0;
    virtual SerialNum getOldestSerialNum() const = 0;
    virtual bool hasValidSerial(SerialNum serialNum) const = 0;
    virtual SerialNum getPrevValidSerial(SerialNum serialNum) const = 0;
    virtual void setProtonConfig(const ProtonConfigSP &protonConfig) = 0;
};

}  // namespace proton

