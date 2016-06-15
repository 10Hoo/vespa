// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * \class storage::ServiceLayerNode
 * \ingroup storageserver
 *
 * \brief Class for setting up a service layer node.
 */

#pragma once

#include <vespa/persistence/spi/persistenceprovider.h>
#include <vespa/storage/bucketdb/minimumusedbitstracker.h>
#include <vespa/storage/storageserver/applicationgenerationfetcher.h>
#include <vespa/storage/storageserver/servicelayernodecontext.h>
#include <vespa/storage/storageserver/storagenode.h>
#include <vespa/config-stor-devices.h>
#include <vespa/config/config.h>

namespace storage {

class FileStorManager;

class ServiceLayerNode
        : public StorageNode,
          private VisitorMessageSessionFactory,
          private config::IFetcherCallback<vespa::config::storage::StorDevicesConfig>

{
    ServiceLayerNodeContext& _context;
    spi::PersistenceProvider& _persistenceProvider;
    spi::PartitionStateList _partitions;
    VisitorFactory::Map _externalVisitors;
    MinimumUsedBitsTracker _minUsedBitsTracker;

    // FIXME: Should probably use the fetcher in StorageNode
    std::unique_ptr<config::ConfigFetcher> _configFetcher;
    std::unique_ptr<vespa::config::storage::StorDevicesConfig> _deviceConfig;
    std::unique_ptr<vespa::config::storage::StorDevicesConfig> _newDevicesConfig;
    FileStorManager* _fileStorManager;
    bool _init_has_been_called;
    bool _noUsablePartitionMode;

public:
    typedef std::unique_ptr<ServiceLayerNode> UP;

    ServiceLayerNode(const config::ConfigUri & configUri,
                            ServiceLayerNodeContext& context,
                            ApplicationGenerationFetcher& generationFetcher,
                            spi::PersistenceProvider& persistenceProvider,
                            const VisitorFactory::Map& externalVisitors);
    ~ServiceLayerNode();
    /**
     * Init must be called exactly once after construction and before destruction.
     */
    void init();

    virtual const lib::NodeType& getNodeType() const
        { return lib::NodeType::STORAGE; }

    virtual ResumeGuard pause();

private:
    virtual void subscribeToConfigs();
    virtual void initializeNodeSpecific();
    virtual void handleLiveConfigUpdate();
    virtual void configure(std::unique_ptr<vespa::config::storage::StorDevicesConfig> config);
    virtual VisitorMessageSession::UP createSession(Visitor&, VisitorThread&);
    virtual documentapi::Priority::Value toDocumentPriority(
            uint8_t storagePriority) const;

    virtual StorageLink::UP createChain();
    virtual void removeConfigSubscriptions();
};

} // storage

