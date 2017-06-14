// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "ilidreusedelayer.h"

namespace searchcorespi
{

namespace index
{

class IThreadingService;

}

}

namespace proton
{

namespace documentmetastore
{

class IStore;

/**
 * This class delays reuse of lids until references to the lids have
 * been purged from the data structures in memory index and attribute
 * vectors.
 *
 * Note that an additional delay is added by the IStore component,
 * where lids are put on a hold list to ensure that queries started
 * before lid was purged also blocks reuse of lid.
 *
 * Currently only works correctly when visibility delay is 0.
 */
class LidReuseDelayer : public ILidReuseDelayer
{
    searchcorespi::index::IThreadingService &_writeService;
    IStore &_documentMetaStore;
    bool _immediateCommit;
    bool _hasIndexedOrAttributeFields;
    std::vector<uint32_t> _pendingLids; // lids waiting for commit

public:
    LidReuseDelayer(searchcorespi::index::IThreadingService &writeService,
                    IStore &documentMetaStore);
    virtual ~LidReuseDelayer();
    virtual bool delayReuse(uint32_t lid) override;
    virtual bool delayReuse(const std::vector<uint32_t> &lids) override;
    virtual void setImmediateCommit(bool immediateCommit) override;
    virtual bool getImmediateCommit() const override;
    virtual void setHasIndexedOrAttributeFields(bool hasIndexedOrAttributeFields) override;
    virtual std::vector<uint32_t> getReuseLids() override;
};

}

}
