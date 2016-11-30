// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/searchlib/attribute/attributefactory.h>
#include <vespa/searchlib/attribute/integerbase.h>
#include <vespa/searchlib/attribute/floatbase.h>
#include <vespa/searchlib/attribute/flagattribute.h>
#include "defines.h"

#include <vespa/log/log.h>
LOG_SETUP(".createarrayfastsearch");

#include <vespa/searchlib/attribute/attributevector.hpp>
#include <vespa/searchlib/attribute/enumstore.hpp>
#include <vespa/searchlib/attribute/enumattribute.hpp>
#include <vespa/searchlib/attribute/multivalueattribute.hpp>
#include <vespa/searchlib/attribute/multienumattribute.hpp>
#include <vespa/searchlib/attribute/multinumericenumattribute.hpp>
#include <vespa/searchlib/attribute/multinumericpostattribute.hpp>
#include <vespa/searchlib/attribute/multistringpostattribute.hpp>

namespace search {

using attribute::BasicType;

#define INTARRAY(T)   MultiValueNumericPostingAttribute< ENUM_ATTRIBUTE(IntegerAttributeTemplate<T>), MULTIVALUE_ENUM_ARG >
#define FLOATARRAY(T) MultiValueNumericPostingAttribute< ENUM_ATTRIBUTE(FloatingPointAttributeTemplate<T>), MULTIVALUE_ENUM_ARG >
#define CREATEINTARRAY(T, fname, info) static_cast<AttributeVector *>(new INTARRAY(T)(fname, info))
#define CREATEFLOATARRAY(T, fname, info) static_cast<AttributeVector *>(new FLOATARRAY(T)(fname, info))

AttributeVector::SP
AttributeFactory::createArrayFastSearch(const vespalib::string & baseFileName, const Config & info)
{
    assert(info.collectionType().type() == attribute::CollectionType::ARRAY);
    assert(info.fastSearch());
    AttributeVector::SP ret;
    switch(info.basicType().type()) {
    case BasicType::UINT1:
    case BasicType::UINT2:
    case BasicType::UINT4:
        break;
    case BasicType::INT8:
        ret.reset(static_cast<AttributeVector *>(new FlagAttribute(baseFileName, info)));
        break;
    case BasicType::INT16:
        ret.reset(CREATEINTARRAY(int16_t, baseFileName, info));
        break;
    case BasicType::INT32:
        ret.reset(CREATEINTARRAY(int32_t, baseFileName, info));
        break;
    case BasicType::INT64:
        ret.reset(CREATEINTARRAY(int64_t, baseFileName, info));
        break;
    case BasicType::FLOAT:
        ret.reset(CREATEFLOATARRAY(float, baseFileName, info));
        break;
    case BasicType::DOUBLE:
        ret.reset(CREATEFLOATARRAY(double, baseFileName, info));
        break;
    case BasicType::STRING:
        ret.reset(static_cast<AttributeVector *>(new ArrayStringPostingAttribute(baseFileName, info)));
        break;
    default:
        break;
    }
    return ret;
}

}
