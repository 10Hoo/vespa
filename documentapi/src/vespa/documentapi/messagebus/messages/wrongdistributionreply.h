// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/documentapi/messagebus/messages/documentreply.h>

namespace documentapi {

class WrongDistributionReply : public DocumentReply {
private:
    string _systemState;

public:
    typedef std::unique_ptr<WrongDistributionReply> UP;
    typedef std::shared_ptr<WrongDistributionReply> SP;

    WrongDistributionReply();

    WrongDistributionReply(const string &systemState);

    const string &getSystemState() const { return _systemState; };

    void setSystemState(const string &state) { _systemState = state; };

    string toString() const { return "wrongdistributionreply"; }
};

}

