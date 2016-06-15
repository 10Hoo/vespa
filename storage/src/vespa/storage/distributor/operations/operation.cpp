// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

/* $Id$ */

#include <vespa/fastos/fastos.h>
#include <vespa/storage/common/distributorcomponent.h>
#include <vespa/storage/distributor/operations/operation.h>
#include <vespa/storageapi/messageapi/storagemessage.h>
#include <vespa/storageapi/messageapi/storagecommand.h>
#include <vespa/storageapi/messageapi/storagereply.h>

#include <vespa/log/log.h>

LOG_SETUP(".distributor.callback");

namespace storage {

namespace distributor {

Operation::Operation()
    : _startTime(0)
{
}

Operation::~Operation()
{
}

std::string
Operation::getStatus() const
{
    return vespalib::make_string("%s (started %s)",
                                 getName(), _startTime.toString().c_str());
}

void
Operation::start(DistributorMessageSender& sender,
                 framework::MilliSecTime startTime)
{
    _startTime = startTime;
    onStart(sender);
}

void
Operation::copyMessageSettings(const api::StorageCommand& source, api::StorageCommand& target)
{
    target.getTrace().setLevel(source.getTrace().getLevel());
    target.setTimeout(source.getTimeout());
    target.setPriority(source.getPriority());
    target.setLoadType(source.getLoadType());
}

}

}
