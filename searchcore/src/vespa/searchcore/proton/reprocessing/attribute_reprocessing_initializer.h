// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "i_reprocessing_handler.h"
#include <vespa/searchcommon/common/schema.h>
#include <vespa/searchcore/proton/attribute/i_attribute_manager.h>
#include <vespa/searchcore/proton/common/i_document_type_inspector.h>
#include <vespa/searchcore/proton/reprocessing/i_reprocessing_initializer.h>
#include <vespa/searchlib/common/serialnum.h>

namespace proton {

/**
 * Class responsible for initialize reprocessing of attribute vectors if needed.
 */
class AttributeReprocessingInitializer : public IReprocessingInitializer
{
public:
    typedef std::unique_ptr<AttributeReprocessingInitializer> UP;

    class Config
    {
    private:
        proton::IAttributeManager::SP _attrMgr;
        const search::index::Schema &_schema;
        IDocumentTypeInspector::SP _inspector;
    public:
        Config(const proton::IAttributeManager::SP &attrMgr,
               const search::index::Schema &schema,
               const IDocumentTypeInspector::SP &inspector)
            : _attrMgr(attrMgr),
              _schema(schema),
              _inspector(inspector)
        {
        }
        const proton::IAttributeManager::SP &getAttrMgr() const { return _attrMgr; }
        const search::index::Schema &getSchema() const { return _schema; }
        const IDocumentTypeInspector::SP &getInspector() const { return _inspector; }
    };

private:
    IReprocessingReader::SP                _attrsToPopulate;
    std::vector<IReprocessingRewriter::SP> _fieldsToPopulate;

public:
    AttributeReprocessingInitializer(const Config &newCfg,
                                     const Config &oldCfg,
                                     const vespalib::string &subDbName);

    // Implements IReprocessingInitializer
    virtual bool hasReprocessors() const;

    virtual void initialize(IReprocessingHandler &handler);
};

} // namespace proton

