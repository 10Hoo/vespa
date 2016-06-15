// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "attributesaver.h"
#include "iattributesavetarget.h"
#include "singleenumattribute.h"
#include "enumattributesaver.h"

namespace search {

/*
 * Class for saving a single value enumerated attribute.
 */
class SingleValueEnumAttributeSaver : public AttributeSaver
{
private:
    using EnumIndexCopyVector =
        SingleValueEnumAttributeBase::EnumIndexCopyVector;
    EnumIndexCopyVector _indices;
    EnumAttributeSaver  _enumSaver;

    virtual bool onSave(IAttributeSaveTarget &saveTarget) override;
public:
    SingleValueEnumAttributeSaver(vespalib::GenerationHandler::Guard &&guard,
                                  const IAttributeSaveTarget::Config &cfg,
                                  EnumIndexCopyVector &&indices,
                                  const EnumStoreBase &enumStore);

    virtual ~SingleValueEnumAttributeSaver();
};

} // namespace search
