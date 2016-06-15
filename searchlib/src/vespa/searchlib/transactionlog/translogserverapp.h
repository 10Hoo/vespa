// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchlib/transactionlog/translogserver.h>
#include <vespa/searchlib/config/config-translogserver.h>
#include <vespa/config/helper/configfetcher.h>
#include <vespa/vespalib/util/ptrholder.h>

namespace search
{

namespace common
{

class FileHeaderContext;

}

namespace transactionlog
{

class TransLogServerApp : public config::IFetcherCallback<searchlib::TranslogserverConfig>
{
private:
    TransLogServer::SP                                   _tls;
    vespalib::PtrHolder<searchlib::TranslogserverConfig>    _tlsConfig;
    config::ConfigFetcher                                _tlsConfigFetcher;
    const common::FileHeaderContext                    & _fileHeaderContext;

    void configure(std::unique_ptr<searchlib::TranslogserverConfig> cfg);

public:
    typedef std::unique_ptr<TransLogServerApp> UP;

    TransLogServerApp(const config::ConfigUri & tlsConfigUri,
                      const common::FileHeaderContext &fileHeaderContext);
    ~TransLogServerApp();

    TransLogServer::SP getTransLogServer() const { return _tls; }

    void start();
};

} // namespace transactionlog
} // namespace search

