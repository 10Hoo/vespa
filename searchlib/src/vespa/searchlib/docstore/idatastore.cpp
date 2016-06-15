// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "idatastore.h"

namespace search {

IDataStore::IDataStore(const vespalib::string& dirName) :
    _nextId(0),
    _dirName(dirName)
{
}

IDataStore::~IDataStore()
{
}


} // namespace search
