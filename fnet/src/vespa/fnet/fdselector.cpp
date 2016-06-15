// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".fnet");
#include <vespa/fnet/fnet.h>


FNET_FDSelector::FNET_FDSelector(FNET_Transport *transport, int fd,
                                 FNET_IFDSelectorHandler *handler,
                                 FNET_Context context)
    : FNET_IOComponent(transport->select_thread(&fd, sizeof(fd)), &_fdSocket, FDSpec(fd).spec(), false),
      _fd(fd),
      _fdSocket(fd),
      _handler(handler),
      _context(context),
      _eventBusy(false),
      _eventWait(false)
{
    AddRef_NoLock();
    Owner()->Add(this, false);
}


void
FNET_FDSelector::updateReadSelection(bool wantRead)
{
    if (wantRead) {
        Owner()->EnableRead(this);
    } else {
        Owner()->DisableRead(this);
    }
}


void
FNET_FDSelector::updateWriteSelection(bool wantWrite)
{
    if (wantWrite) {
        Owner()->EnableWrite(this);
    } else {
        Owner()->DisableWrite(this);
    }
}


void
FNET_FDSelector::dispose()
{
    Lock();
    waitEvent();
    _handler = NULL;
    Unlock();
    Owner()->Close(this, false);
}


FNET_FDSelector::~FNET_FDSelector()
{
    assert(_fdSocket.GetSocketEvent() == NULL);
}


void
FNET_FDSelector::Close()
{
    SetSocketEvent(NULL);
}


bool
FNET_FDSelector::HandleReadEvent()
{
    if (!_flags._ioc_readEnabled) {
        return true;
    }
    Lock();
    FNET_IFDSelectorHandler *handler = _handler;
    beforeEvent();
    if (handler != NULL) {
        handler->readEvent(this);
    }
    afterEvent();
    Unlock();
    return true;
}


bool
FNET_FDSelector::HandleWriteEvent()
{
    if (!_flags._ioc_writeEnabled) {
        return true;
    }
    Lock();
    FNET_IFDSelectorHandler *handler = _handler;
    beforeEvent();
    if (handler != NULL) {
        handler->writeEvent(this);
    }
    afterEvent();
    Unlock();
    return true;
}
