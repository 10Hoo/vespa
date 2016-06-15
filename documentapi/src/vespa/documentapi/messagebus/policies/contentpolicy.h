// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/documentapi/messagebus/policies/storagepolicy.h>

namespace documentapi {

class ContentPolicy : public StoragePolicy
{
public:
    ContentPolicy(const string& param);
private:
    virtual string createConfigId(const string & clusterName) const;
};

}

