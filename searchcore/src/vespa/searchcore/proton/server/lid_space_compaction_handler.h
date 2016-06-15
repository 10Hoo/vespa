// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "i_lid_space_compaction_handler.h"
#include "idocumentsubdb.h"

namespace proton {

/**
 * Class that handles lid space compaction over a single document sub db.
 */
class LidSpaceCompactionHandler : public ILidSpaceCompactionHandler
{
private:
    IDocumentSubDB  &_subDb;
    vespalib::string _docTypeName;

public:
    LidSpaceCompactionHandler(IDocumentSubDB &subDb,
                              const vespalib::string &docTypeName);

    // Implements ILidSpaceCompactionHandler
    virtual vespalib::string getName() const {
        return _docTypeName + "." + _subDb.getName();
    }
    virtual uint32_t getSubDbId() const { return _subDb.getSubDbId(); }
    virtual search::LidUsageStats getLidStatus() const;
    virtual IDocumentScanIterator::UP getIterator() const;
    virtual MoveOperation::UP createMoveOperation(const search::DocumentMetaData &document, uint32_t moveToLid) const;
    virtual void handleMove(const MoveOperation &op);
    virtual void handleCompactLidSpace(const CompactLidSpaceOperation &op);
};

} // namespace proton

