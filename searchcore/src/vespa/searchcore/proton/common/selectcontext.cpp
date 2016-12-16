// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "selectcontext.h"
#include "cachedselect.h"
#include <vespa/document/select/value.h>
#include <vespa/searchlib/attribute/attributeguard.h>

namespace proton {

using document::select::Value;
using document::select::Context;
using search::AttributeGuard;
using search::AttributeVector;

namespace select {
    struct Guards : public std::vector<AttributeGuard> {
        using Parent = std::vector<AttributeGuard>;
        using Parent::Parent;
    };
}

SelectContext::SelectContext(const CachedSelect &cachedSelect)
    : Context(),
      _docId(0u),
      _guards(std::make_unique<select::Guards>(cachedSelect._attributes.size())),
      _cachedSelect(cachedSelect)
{ }

SelectContext::~SelectContext() { }

void
SelectContext::getAttributeGuards(void)
{
    _guards->resize(_cachedSelect._attributes.size());
    std::vector<AttributeVector::SP>::const_iterator j(_cachedSelect._attributes.begin());
    for (std::vector<AttributeGuard>::iterator i(_guards->begin()), ie(_guards->end()); i != ie; ++i, ++j) {
        *i = AttributeGuard(*j);
    }
}


void
SelectContext::dropAttributeGuards(void)
{
    _guards->clear();
}

}
