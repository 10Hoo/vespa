// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchcommon/attribute/iattributecontext.h>
#include <vespa/vespalib/util/sync.h>
#include <vespa/vespalib/stllike/hash_map.h>
#include "iattributemanager.h"

namespace search {

/**
 * This class is wrapping an attribute manager and
 * implements the IAttributeContext interface to provide read access to attribute vectors.
 **/
class AttributeContext : public attribute::IAttributeContext
{
private:
    typedef vespalib::hash_map<string, AttributeGuard::UP> AttributeMap;

    const search::IAttributeManager & _manager;
    mutable AttributeMap              _attributes;
    mutable AttributeMap              _enumAttributes;
    mutable vespalib::Lock            _cacheLock;

    const attribute::IAttributeVector *
        getAttribute(AttributeMap & map, const string & name, bool stableEnum) const;

public:
    AttributeContext(const search::IAttributeManager & manager);

    // Implements IAttributeContext
    const attribute::IAttributeVector * getAttribute(const string & name) const override;
    const attribute::IAttributeVector * getAttributeStableEnum(const string & name) const override;
    void getAttributeList(std::vector<const attribute::IAttributeVector *> & list) const override;
    void releaseEnumGuards() override;

    // Give acces to the underlying manager
    const search::IAttributeManager & getManager() const { return _manager; }
};

} // namespace search

