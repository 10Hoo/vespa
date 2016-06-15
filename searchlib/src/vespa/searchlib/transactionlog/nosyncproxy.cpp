// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include "nosyncproxy.h"

namespace search
{
namespace transactionlog
{

NoSyncProxy::NoSyncProxy(void)
{
}


NoSyncProxy::~NoSyncProxy(void)
{
}


void
NoSyncProxy::sync(SerialNum syncTo)
{
    (void) syncTo;
}

}

}
