// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".proton.server.flushhandlerproxy");
#include "flushhandlerproxy.h"

namespace proton {

FlushHandlerProxy::FlushHandlerProxy(const DocumentDB::SP &documentDB)
    : IFlushHandler(documentDB->getDocTypeName().toString()),
      _documentDB(documentDB)
{
    _documentDB->retain();
}


FlushHandlerProxy::~FlushHandlerProxy(void)
{
    _documentDB->release();
}


std::vector<IFlushTarget::SP>
FlushHandlerProxy::getFlushTargets(void)
{
    return _documentDB->getFlushTargets();
}


IFlushHandler::SerialNum
FlushHandlerProxy::getCurrentSerialNumber(void) const
{
    return _documentDB->getCurrentSerialNumber();
}


void
FlushHandlerProxy::flushDone(SerialNum oldestSerial)
{
    _documentDB->flushDone(oldestSerial);
}


void
FlushHandlerProxy::syncTls(SerialNum syncTo)
{
    _documentDB->sync(syncTo);
}


} // namespace proton
