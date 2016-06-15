// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "fileheadercontext.h"
#include "serialnum.h"

namespace search
{

namespace common
{

class SerialNumFileHeaderContext : public FileHeaderContext
{
    const FileHeaderContext &_parentFileHeaderContext;
    SerialNum _serialNum;

public:
    SerialNumFileHeaderContext(const FileHeaderContext &
                               parentFileHeaderContext,
                               SerialNum serialNum);

    virtual void
    addTags(vespalib::GenericHeader &header,
            const vespalib::string &name) const;
};

} // namespace common

} // namespace search

