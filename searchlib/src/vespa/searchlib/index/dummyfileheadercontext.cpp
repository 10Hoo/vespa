// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".index.dummyfileheadercontext");
#include "dummyfileheadercontext.h"
#include <vespa/vespalib/data/fileheader.h>
#include <vespa/searchlib/util/fileheadertk.h>

namespace search
{

namespace index
{

vespalib::string DummyFileHeaderContext::_creator;

DummyFileHeaderContext::DummyFileHeaderContext(void)
    : common::FileHeaderContext(),
      _disableFileName(false),
      _hostName(),
      _pid(getpid())
{
    _hostName = FastOS_Socket::getHostName();
    assert(!_hostName.empty());
}


DummyFileHeaderContext::~DummyFileHeaderContext(void)
{
}


void
DummyFileHeaderContext::disableFileName(void)
{
    _disableFileName = true;
}


void
DummyFileHeaderContext::addTags(vespalib::GenericHeader &header,
                                const vespalib::string &name) const
{
    typedef vespalib::GenericHeader::Tag Tag;

    FileHeaderTk::addVersionTags(header);
    if (!_disableFileName) {
        header.putTag(Tag("fileName", name));
        addCreateAndFreezeTime(header);
    }
    header.putTag(Tag("hostName", _hostName));
    header.putTag(Tag("pid", _pid));
    if (!_creator.empty()) {
        header.putTag(Tag("creator", _creator));
    }
    header.putTag(Tag("DummyFileHeaderContext", "enabled"));
}


void
DummyFileHeaderContext::setCreator(const vespalib::string &creator)
{
    _creator = creator;
}


} // namespace index

} // namespace search
