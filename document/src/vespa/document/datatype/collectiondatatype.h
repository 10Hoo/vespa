// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * \class document::CollectionDataType
 * \ingroup datatype
 *
 * \brief Data type used for collections of data with similar types.
 *
 * This contains common functionality for array and weighted set datatypes.
 */
#pragma once

#include <vespa/document/datatype/datatype.h>

namespace document {

class CollectionDataType : public DataType {
    const DataType *_nestedType;

protected:
    CollectionDataType() : _nestedType(0) {}
    CollectionDataType(const CollectionDataType&);
    CollectionDataType& operator=(const CollectionDataType&);
    CollectionDataType(const vespalib::stringref & name,
                       const DataType &nestedType);
    CollectionDataType(const vespalib::stringref & name,
                       const DataType &nestedType, int32_t id);

public:
    virtual ~CollectionDataType();

    bool operator==(const DataType&) const;

    const DataType &getNestedType() const { return *_nestedType; }

    DECLARE_IDENTIFIABLE_ABSTRACT(CollectionDataType);
};

} // document


