// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/storageframework/storageframework.h>

namespace storage {
namespace distributor {

struct DelegatedStatusRequest
{
    const framework::StatusReporter& reporter;
    const framework::HttpUrlPath& path;
    std::ostream& outputStream;

    DelegatedStatusRequest(const framework::StatusReporter& _reporter,
                     const framework::HttpUrlPath& _path,
                     std::ostream& _outputStream)
      : reporter(_reporter),
        path(_path),
        outputStream(_outputStream)
    {}

private:
    DelegatedStatusRequest(const DelegatedStatusRequest&);
    DelegatedStatusRequest& operator=(const DelegatedStatusRequest&);
};

} // distributor
} // storage
