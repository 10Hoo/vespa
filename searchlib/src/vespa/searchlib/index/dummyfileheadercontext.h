// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/common/fileheadercontext.h>

namespace vespalib
{

class GenericHeader;

}

namespace search
{

namespace index
{

class DummyFileHeaderContext : public common::FileHeaderContext
{
    bool _disableFileName;
    vespalib::string _hostName;
    pid_t _pid;

    static vespalib::string _creator;
public:
    DummyFileHeaderContext(void);

    virtual
    ~DummyFileHeaderContext(void);

    void
    disableFileName(void);

    virtual void
    addTags(vespalib::GenericHeader &header,
            const vespalib::string &name) const;

    static void
    setCreator(const vespalib::string &creator);
};


} // namespace index

} // namespace search

