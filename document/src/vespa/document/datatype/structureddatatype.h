// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * \class document::StructuredDataType
 * \ingroup datatype
 *
 * \brief Data type describing common parts for structured datatypes.
 *
 * This class contains common functionality for structured data types, like
 * structs and documents.
 */
#pragma once

#include <vespa/document/base/field.h>
#include <vespa/document/datatype/datatype.h>
#include <vespa/fastos/fastos.h> // Get uint16_t type on linux
#include <set>
#include <vespa/vespalib/util/exception.h>

namespace document {

class StructuredDataType : public DataType {
    virtual std::unique_ptr<FieldPath> onBuildFieldPath(const vespalib::stringref & remainFieldName) const;

protected:
    StructuredDataType();
    StructuredDataType(const vespalib::stringref & name);
    StructuredDataType(const vespalib::stringref & name, int32_t dataTypeId);


public:
    virtual uint32_t getFieldCount() const = 0;

    /** @throws FieldNotFoundException if field does not exist. */
    virtual const Field& getField(const vespalib::stringref & name) const = 0;

    /** @throws FieldNotFoundException if field does not exist. */
    virtual const Field& getField(int fieldId, int version) const = 0;

    virtual bool hasField(const vespalib::stringref & name) const = 0;
    virtual bool hasField(int32_t fieldId, int version) const = 0;

    virtual Field::Set getFieldSet() const = 0;

        // Implementation of DataType
    virtual StructuredDataType* clone() const = 0;
    virtual bool operator==(const DataType& type) const;

    static int32_t createId(const vespalib::stringref &name);

    DECLARE_IDENTIFIABLE_ABSTRACT(StructuredDataType);

};

}

