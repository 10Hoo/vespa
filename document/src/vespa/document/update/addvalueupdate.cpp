// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include "addvalueupdate.h"
#include <vespa/document/base/field.h>
#include <vespa/document/datatype/arraydatatype.h>
#include <vespa/document/datatype/weightedsetdatatype.h>
#include <vespa/document/fieldvalue/fieldvalues.h>
#include <vespa/document/repo/fixedtyperepo.h>
#include <vespa/document/serialization/vespadocumentdeserializer.h>
#include <vespa/document/util/serializable.h>
#include <vespa/document/util/serializableexceptions.h>
#include <vespa/vespalib/objects/nbostream.h>

using vespalib::IllegalArgumentException;
using vespalib::IllegalStateException;
using vespalib::nbostream;

namespace document
{

IMPLEMENT_IDENTIFIABLE(AddValueUpdate, ValueUpdate);

bool
AddValueUpdate::operator==(const ValueUpdate& other) const
{
    if (other.getClass().id() != AddValueUpdate::classId) return false;
    const AddValueUpdate& o(static_cast<const AddValueUpdate&>(other));
    if (*_value != *o._value) return false;
    if (_weight != o._weight) return false;
    return true;
}

// Ensure that this update is compatible with given field.
void
AddValueUpdate::checkCompatibility(const Field& field) const
{
    if (field.getDataType().inherits(CollectionDataType::classId)) {
        const CollectionDataType& type(
                static_cast<const CollectionDataType&>(field.getDataType()));
        if (!type.getNestedType().isValueType(*_value)) {
            throw IllegalArgumentException(
                "Cannot add value of type "
                + _value->getDataType()->toString() + " to field "
                + field.getName().c_str() + " of container type "
                + field.getDataType().toString(), VESPA_STRLOC);
        }
    } else {
        throw IllegalArgumentException(
            "Can not add a value to field of type"
            + field.getDataType().toString(), VESPA_STRLOC);
    }
}

// Print this update in human readable form.
void
AddValueUpdate::print(std::ostream& out, bool, const std::string& indent) const
{
    out << indent << "AddValueUpdate(" << *_value << ", " << _weight << ")";
}

// Apply this update to the given document.
bool
AddValueUpdate::applyTo(FieldValue& value) const
{
    if (value.inherits(ArrayFieldValue::classId)) {
        ArrayFieldValue& doc(static_cast<ArrayFieldValue&>(value));
        doc.add(*_value);	
    } else if (value.inherits(WeightedSetFieldValue::classId)) {
        WeightedSetFieldValue& doc(static_cast<WeightedSetFieldValue&>(value));
        doc.add(*_value, _weight);	
    } else {
        std::string err = vespalib::make_string(
                "Unable to add a value to a \"%s\" field value.",
                value.getClass().name());
        throw IllegalStateException(err, VESPA_STRLOC);
    }
    return true;
}

void
AddValueUpdate::printXml(XmlOutputStream& xos) const
{
    xos << XmlTag("add") << XmlAttribute("weight", _weight)
        << *_value
        << XmlEndTag();
}

// Deserialize this update from the given buffer.
void
AddValueUpdate::deserialize(const DocumentTypeRepo& repo,
                            const DataType& type,
                            ByteBuffer& buffer, uint16_t version)
{
    const CollectionDataType* ctype =
        Identifiable::cast<const CollectionDataType*>(&type);
    if (ctype == NULL) {
        throw DeserializeException("Can not perform add operation on "
                                   "non-collection type.");
    }
    _value.reset(ctype->getNestedType().createFieldValue().release());
    nbostream stream(buffer.getBufferAtPos(), buffer.getRemaining());
    VespaDocumentDeserializer deserializer(repo, stream, version);
    deserializer.read(*_value);
    buffer.incPos(buffer.getRemaining() - stream.size());
    buffer.getIntNetwork(_weight);
}

}

