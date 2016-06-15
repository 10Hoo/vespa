// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/document/base/field.h>

#include <vespa/document/datatype/datatype.h>
#include <vespa/vespalib/util/exceptions.h>
#include <vespa/vespalib/stllike/asciistream.h>
#include <vespa/vespalib/util/bobhash.h>

namespace document {

Field::Field(const vespalib::stringref & name, int fieldId,
             const DataType& dataType, bool headerField)
    : FieldBase(name),
      _dataType(&dataType),
      _fieldIdV6(fieldId),
      _fieldId(fieldId),
      _isHeaderField(headerField)
{
}

Field::Field(const vespalib::stringref & name, int fieldId, int fieldIdV6,
             const DataType& dataType, bool headerField)
    : FieldBase(name),
      _dataType(&dataType),
      _fieldIdV6(fieldIdV6),
      _fieldId(fieldId),
      _isHeaderField(headerField)
{
}

Field::Field(const vespalib::stringref & name,
             const DataType& dataType, bool headerField)
    : FieldBase(name),
      _dataType(&dataType),
      _fieldIdV6(calculateIdV6()),
      _fieldId(calculateIdV7()),
      _isHeaderField(headerField)
{
}

vespalib::string
Field::toString(bool verbose) const
{
    vespalib::asciistream out;
    out << "Field(" << getName();
    if (verbose) {
        out << ", id " << _fieldId;
    }
    out << ", " << _dataType->toString();
    if (verbose) {
        out << ", " << (_isHeaderField ? "header" : "body");
    }
    out << ")";
    return out.str();
}

bool
Field::contains(const FieldSet& fields) const
{
    switch (fields.getType()) {
    case FIELD:
        return static_cast<const Field&>(fields).getId(7) == getId(7);
    case SET:
    {
        // Go through each.
        return false;
    }
    case NONE:
    case DOCID:
        return true;
    case HEADER:
    case BODY:
    case ALL:
        return false;
    }

    return false;
}

int
Field::calculateIdV6()
{
    int newId =
        vespalib::BobHash::hash(getName().c_str(), getName().size(), 0);
        // Highest bit is reserved to tell 7-bit id's from 31-bit ones
    if (newId < 0) newId = -newId;
    validateId(newId);
    return newId;
}

int
Field::calculateIdV7()
{
    vespalib::asciistream ost;
    ost << getName();
    ost << _dataType->getId();

    int newId = vespalib::BobHash::hash(
            ost.str().c_str(), ost.str().length(), 0);
        // Highest bit is reserved to tell 7-bit id's from 31-bit ones
    if (newId < 0) newId = -newId;
    validateId(newId);
    return newId;
}

void
Field::validateId(int newId) {
    if (newId >= 100 && newId <= 127) {
        throw vespalib::IllegalArgumentException(vespalib::make_string(
                    "Attempt to set the id of %s to %d failed, values from "
                    "100 to 127 are reserved for internal use",
                    getName().c_str(), newId));
    }

    if ((newId & 0x80000000) != 0) // Highest bit must not be set
    {
        throw vespalib::IllegalArgumentException(vespalib::make_string(
                    "Attempt to set the id of %s to %d"
                    " failed, negative id values are illegal",
                    getName().c_str(), newId));
    }
}

} // document
