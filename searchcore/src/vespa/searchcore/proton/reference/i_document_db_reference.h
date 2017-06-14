// Copyright 2017 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/vespalib/stllike/string.h>

namespace search {

class AttributeVector;
class IGidToLidMapperFactory;

}

namespace proton {

class GidToLidChangeRegistrator;

/*
 * Interface class for getting target attributes for imported
 * attributes, and for getting interface for mapping to lids
 * compatible with the target attributes.
 */
class IDocumentDBReference
{
public:
    using SP = std::shared_ptr<IDocumentDBReference>;
    virtual ~IDocumentDBReference() { }
    virtual std::shared_ptr<search::AttributeVector> getAttribute(vespalib::stringref name) = 0;
    virtual std::shared_ptr<search::IGidToLidMapperFactory> getGidToLidMapperFactory() = 0;
    virtual std::unique_ptr<GidToLidChangeRegistrator> makeGidToLidChangeRegistrator(const vespalib::string &docTypeName) = 0;
};

} // namespace proton
