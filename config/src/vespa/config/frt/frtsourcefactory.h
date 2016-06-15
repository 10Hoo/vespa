// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/config/common/sourcefactory.h>
#include <vespa/config/common/timingvalues.h>
#include "connectionfactory.h"
#include "frtconfigrequestfactory.h"

namespace config {

/**
 * Class for sending and receiving config requests via FRT.
 */
class FRTSourceFactory : public SourceFactory
{
public:
    FRTSourceFactory(ConnectionFactory::UP connectionFactory, const TimingValues & timingValues, int protocolVersion, int traceLevel, const VespaVersion & vespaVersion, const CompressionType & compressionType);

    /**
     * Create source handling config described by key.
     */
    Source::UP createSource(const IConfigHolder::SP & holder, const ConfigKey & key) const;

private:
    ConnectionFactory::SP _connectionFactory;
    FRTConfigRequestFactory _requestFactory;
    const TimingValues _timingValues;
};

} // namespace config

