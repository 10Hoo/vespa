// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".config.file.filesource");
#include "filesource.h"
#include <vespa/config/subscription/sourcespec.h>
#include <vespa/config/common/misc.h>
#include <vespa/vespalib/io/fileutil.h>
#include <vespa/vespalib/stllike/asciistream.h>

using vespalib::asciistream;

namespace config {

FileSource::FileSource(const IConfigHolder::SP & holder, const vespalib::string & fileName)
    : _holder(holder),
      _fileName(fileName),
      _lastLoaded(-1),
      _generation(1)
{
}

void
FileSource::getConfig()
{
    std::vector<vespalib::string> lines(readConfigFile(_fileName));
    int64_t last = getLast(_fileName);

    if (last > _lastLoaded) {
        _holder->handle(ConfigUpdate::UP(new ConfigUpdate(ConfigValue(lines, calculateContentMd5(lines)), true, _generation)));
        _lastLoaded = last;
    } else {
        _holder->handle(ConfigUpdate::UP(new ConfigUpdate(ConfigValue(lines, calculateContentMd5(lines)), false, _generation)));
    }
}

void
FileSource::reload(int64_t generation)
{
    _generation = generation;
}

int64_t
FileSource::getLast(const vespalib::string & fileName)
{
    struct stat filestat;
    memset(&filestat, 0, sizeof(filestat));
    stat(fileName.c_str(), &filestat);
    return filestat.st_mtime;
}

std::vector<vespalib::string>
FileSource::readConfigFile(const vespalib::string & fileName)
{
    asciistream is(asciistream::createFromFile(fileName));
    return is.getlines();
}

void
FileSource::close()
{
}

} // namespace config
