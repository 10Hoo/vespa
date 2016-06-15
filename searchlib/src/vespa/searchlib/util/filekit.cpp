// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Copyright (C) 1998-2003 Fast Search & Transfer ASA
// Copyright (C) 2003 Overture Services Norway AS

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
#include <vespa/searchlib/util/filekit.h>
#include <vespa/vespalib/util/error.h>
#include <memory>
#include <string>
LOG_SETUP(".filekit");

namespace search
{

using vespalib::getLastErrorString;

bool
FileKit::createStamp(const vespalib::stringref &name)
{
    FastOS_File stamp;
    FastOS_StatInfo statInfo;
    bool statres;

    statres = FastOS_File::Stat(name.c_str(), &statInfo);

    if (!statres && (statInfo._error != FastOS_StatInfo::FileNotFound)) {
        LOG(error, "FATAL: Could not check stamp file %s: %s",
            name.c_str(), getLastErrorString().c_str());
        return false;
    }
    if (statres && statInfo._size > 0) {
        LOG(error, "FATAL: Stamp file not empty: %s", name.c_str());
        return false;
    }

    if (!stamp.OpenWriteOnlyTruncate(name.c_str())) {
        LOG(error, "FATAL: Could not create stamp file %s: %s",
            name.c_str(), getLastErrorString().c_str());
        return false;
    }
    return true;
}


bool
FileKit::hasStamp(const vespalib::stringref &name)
{
    FastOS_StatInfo statInfo;
    bool statres;

    statres = FastOS_File::Stat(name.c_str(), &statInfo);

    if (!statres && (statInfo._error != FastOS_StatInfo::FileNotFound)) {
        LOG(error, "FATAL: Could not check stamp file %s: %s",
            name.c_str(), getLastErrorString().c_str());
        return false;
    }
    return statres;
}


bool
FileKit::removeStamp(const vespalib::stringref &name)
{
    FastOS_StatInfo statInfo;
    bool deleteres;
    bool statres;

    statres = FastOS_File::Stat(name.c_str(), &statInfo);

    if (!statres && (statInfo._error != FastOS_StatInfo::FileNotFound)) {
        LOG(error, "FATAL: Could not check stamp file %s: %s",
            name.c_str(), getLastErrorString().c_str());
        return false;
    }
    if (statres && statInfo._size > 0) {
        LOG(error, "FATAL: Stamp file not empty: %s", name.c_str());
        return false;
    }

    do {
        deleteres = FastOS_File::Delete(name.c_str());
        //FIX! errno
    } while (!deleteres && errno == EINTR);

    if (!deleteres &&
        FastOS_File::GetLastError() != FastOS_File::ERR_ENOENT) {
        LOG(error, "FATAL: Could not remove stamp file %s: %s",
            name.c_str(), getLastErrorString().c_str());
        return false;
    }
    return true;
}


fastos::TimeStamp
FileKit::getModificationTime(const vespalib::stringref &name)
{
    FastOS_StatInfo statInfo;
    if (FastOS_File::Stat(name.c_str(), &statInfo)) {
        return fastos::TimeStamp(statInfo._modifiedTimeNS);
    }
    return fastos::TimeStamp();
}


}
