// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".simplereply");
#include "simplereply.h"
#include "simpleprotocol.h"

namespace mbus {

SimpleReply::SimpleReply(const string &str) :
    Reply(),
    _value(str)
{
    // empty
}

SimpleReply::~SimpleReply()
{
    // empty
}

void
SimpleReply::setValue(const string &value)
{
    _value = value;
}

const string &
SimpleReply::getValue() const
{
    return _value;
}

const string &
SimpleReply::getProtocol() const
{
    return SimpleProtocol::NAME;
}

uint32_t
SimpleReply::getType() const
{
    return SimpleProtocol::REPLY;
}

} // namespace mbus
