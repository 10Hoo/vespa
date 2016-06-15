// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * \class storage::ServiceLayerProcess
 *
 * \brief A process running a service layer.
 */
/**
 * \class storage::MemFileServiceLayerProcess
 *
 * \brief A process running a service layer with memfile persistence provider.
 */
/**
 * \class storage::RpcServiceLayerProcess
 *
 * \brief A process running a service layer with RPC persistence provider.
 */
#pragma once

#include <vespa/storageserver/app/process.h>
#include <vespa/config/config.h>
#include <vespa/config/helper/configfetcher.h>
#include <vespa/config-persistence.h>

namespace storage {

class ServiceLayerProcess : public Process {
    VisitorFactory::Map _externalVisitors;
    ServiceLayerNode::UP _node;

protected:
    ServiceLayerNodeContext _context;

public:
    ServiceLayerProcess(const config::ConfigUri & configUri);

    virtual void shutdown();

    virtual void setupProvider() = 0;
    virtual spi::PersistenceProvider& getProvider() = 0;

    virtual void createNode();

    virtual StorageNode& getNode() { return *_node; }
    virtual StorageNodeContext& getContext() { return _context; }

    virtual std::string getComponentName() const { return "servicelayer"; }
};

} // storage

