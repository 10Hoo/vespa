// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchlib/queryeval/irequestcontext.h>
#include <vespa/searchcommon/attribute/iattributecontext.h>

namespace proton {

class RequestContext : public search::queryeval::IRequestContext
{
public:
    using IAttributeContext = search::attribute::IAttributeContext;
    using Doom = vespalib::Doom;
    RequestContext(const Doom & softDoom, const Doom & doom, IAttributeContext & attributeContext);
    const Doom & getSoftDoom() const override { return _softDoom; }
    const Doom & getDoom() const override { return _doom; }
    const search::AttributeVector * getAttribute(const vespalib::string & name) const override;
    const search::AttributeVector * getAttributeStableEnum(const vespalib::string & name) const override;
private:
    const Doom          _softDoom;
    const Doom          _doom;
    IAttributeContext & _attributeContext;
};

}
