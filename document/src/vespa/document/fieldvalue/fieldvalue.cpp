// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/document/fieldvalue/fieldvalue.h>

#include <vespa/document/base/exceptions.h>
#include <vespa/document/datatype/arraydatatype.h>
#include <vespa/document/datatype/numericdatatype.h>
#include <vespa/document/datatype/primitivedatatype.h>
#include <vespa/document/fieldvalue/arrayfieldvalue.h>
#include <vespa/document/fieldvalue/intfieldvalue.h>
#include <vespa/document/fieldvalue/floatfieldvalue.h>
#include <vespa/document/fieldvalue/stringfieldvalue.h>
#include <vespa/document/fieldvalue/rawfieldvalue.h>
#include <vespa/document/fieldvalue/longfieldvalue.h>
#include <vespa/document/fieldvalue/doublefieldvalue.h>
#include <vespa/document/fieldvalue/bytefieldvalue.h>
#include <vespa/document/fieldvalue/predicatefieldvalue.h>
#include <vespa/document/serialization/vespadocumentserializer.h>
#include <vespa/document/util/bytebuffer.h>
#include <vespa/vespalib/objects/fieldbase.h>
#include <vespa/vespalib/objects/nbostream.h>

using vespalib::FieldBase;
using vespalib::nbostream;

namespace document {

IMPLEMENT_IDENTIFIABLE_ABSTRACT(FieldValue, vespalib::Identifiable);

void FieldValue::serialize(nbostream &stream) const {
    VespaDocumentSerializer serializer(stream);
    serializer.write(*this);
}

void FieldValue::serialize(ByteBuffer& buffer) const {
    nbostream stream;
    serialize(stream);
    buffer.putBytes(stream.peek(), stream.size());
}

std::unique_ptr<ByteBuffer> FieldValue::serialize() const {
    nbostream stream;
    serialize(stream);

    std::unique_ptr<ByteBuffer> retVal(new ByteBuffer(stream.size()));
    retVal->putBytes(stream.peek(), stream.size());
    return retVal;
}

size_t
FieldValue::hash() const
{
    vespalib::nbostream os;
    serialize(os);
    return vespalib::hashValue(os.c_str(), os.size()) ;
}

FieldValue&
FieldValue::assign(const FieldValue& value)
{
    throw vespalib::IllegalArgumentException(
            "Cannot assign value of type " + value.getDataType()->toString()
            + " to value of type " + value.getDataType()->toString(), VESPA_STRLOC);
}

/**
 * Normally, tag names are decided by the parent node, so children will not
 * print their parents. For toXML to work on non-root (non-document) nodes
 * though, we've overwritten toXml to write a start tag for leaf nodes, if
 * they are used as root nodes in toXml call.
 */
std::string
FieldValue::toXml(const std::string& indent) const
{
    std::ostringstream ost;
    XmlOutputStream xos(ost, indent);
    xos << XmlTag("value");
    printXml(xos);
    xos << XmlEndTag();
    return ost.str();
}

// Subtypes should implement the conversion functions that make sense

FieldValue& FieldValue::operator=(const vespalib::stringref &)
{
    throw vespalib::IllegalArgumentException(
            "Cannot assign string to datatype " + getDataType()->toString(),
            VESPA_STRLOC);
}

FieldValue& FieldValue::operator=(int32_t)
{
    throw vespalib::IllegalArgumentException(
            "Cannot assign int to datatype " + getDataType()->toString(),
            VESPA_STRLOC);
}

FieldValue& FieldValue::operator=(int64_t)
{
    throw vespalib::IllegalArgumentException(
            "Cannot assign long to datatype " + getDataType()->toString(),
            VESPA_STRLOC);
}

FieldValue& FieldValue::operator=(float)
{
    throw vespalib::IllegalArgumentException(
            "Cannot assign float to datatype " + getDataType()->toString(),
            VESPA_STRLOC);
}

FieldValue& FieldValue::operator=(double)
{
    throw vespalib::IllegalArgumentException(
            "Cannot assign double to datatype " + getDataType()->toString(),
            VESPA_STRLOC);
}

char FieldValue::getAsByte() const
{
    throw InvalidDataTypeConversionException(
            *getDataType(), *DataType::BYTE, VESPA_STRLOC);
}

int32_t FieldValue::getAsInt() const
{
    throw InvalidDataTypeConversionException(
            *getDataType(), *DataType::INT, VESPA_STRLOC);
}

int64_t FieldValue::getAsLong() const
{
    throw InvalidDataTypeConversionException(
            *getDataType(), *DataType::LONG, VESPA_STRLOC);
}

float FieldValue::getAsFloat() const
{
    throw InvalidDataTypeConversionException(
            *getDataType(), *DataType::FLOAT, VESPA_STRLOC);
}

double FieldValue::getAsDouble() const
{
    throw InvalidDataTypeConversionException(
            *getDataType(), *DataType::DOUBLE, VESPA_STRLOC);
}

vespalib::string FieldValue::getAsString() const
{
    throw InvalidDataTypeConversionException(
            *getDataType(), *DataType::STRING, VESPA_STRLOC);
}

std::pair<const char*, size_t> FieldValue::getAsRaw() const
{
    throw InvalidDataTypeConversionException(
            *getDataType(), *DataType::RAW, VESPA_STRLOC);
}

FieldValue::UP FieldValue::getNestedFieldValue(FieldPath::const_iterator start, FieldPath::const_iterator end) const
{
    return (start != end) ? onGetNestedFieldValue(start, end) : FieldValue::UP();
}

FieldValue::UP FieldValue::onGetNestedFieldValue(FieldPath::const_iterator start, FieldPath::const_iterator end) const
{
    (void) start;
    (void) end;
    return FieldValue::UP();
}

FieldValue::IteratorHandler::ModificationStatus
FieldValue::iterateNested(FieldPath::const_iterator start, FieldPath::const_iterator end, IteratorHandler & handler) const
{
    return onIterateNested(start, end, handler);
}

FieldValue::IteratorHandler::ModificationStatus
FieldValue::onIterateNested(FieldPath::const_iterator start, FieldPath::const_iterator end, IteratorHandler & handler) const
{
    if (start == end) {
        handler.handlePrimitive(*this);
        return handler.modify(const_cast<FieldValue&>(*this));
    } else {
        throw vespalib::IllegalArgumentException("Primitive types can't be iterated through");
    }
}

bool
FieldValue::IteratorHandler::IndexValue::operator==(const FieldValue::IteratorHandler::IndexValue& other) const {
    if (key.get() != NULL) {
        if (other.key.get() != NULL && *key == *other.key) {
            return true;
        }
        return false;
    }

    return index == other.index;
}

void FieldValue::IteratorHandler::handlePrimitive(const FieldValue & fv) { onPrimitive(Content(fv, getWeight())); }
bool FieldValue::IteratorHandler::handleComplex(const FieldValue & fv) { return onComplex(Content(fv, getWeight())); }
void FieldValue::IteratorHandler::handleCollectionStart(const FieldValue & fv) { onCollectionStart(Content(fv, getWeight())); }
void FieldValue::IteratorHandler::handleCollectionEnd(const FieldValue & fv) { onCollectionEnd(Content(fv, getWeight())); }
void FieldValue::IteratorHandler::handleStructStart(const FieldValue & fv) { onStructStart(Content(fv, getWeight())); }
void FieldValue::IteratorHandler::handleStructEnd(const FieldValue & fv) { onStructEnd(Content(fv, getWeight())); }

std::string
FieldValue::IteratorHandler::toString(const VariableMap& vars) {
    std::ostringstream out;
    out << "[ ";
    for (FieldValue::IteratorHandler::VariableMap::const_iterator iter = vars.begin();
         iter != vars.end();
         iter++) {
        out << iter->first << "=" << iter->second.toString() << " ";
    }
    out << "]";
    return out.str();
}

std::string
FieldValue::toString(bool verbose, const std::string& indent) const
{
    std::ostringstream o;
    print(o, verbose, indent);
    return o.str();
}

using vespalib::ComplexArrayT;
using vespalib::PrimitiveArrayT;

namespace {
class FieldValueFactory : public ComplexArrayT<FieldValue>::Factory
{
public:
    FieldValueFactory(DataType::UP dataType) : _dataType(dataType.release()) { }
    FieldValue * create() { return _dataType->createFieldValue().release(); }
    virtual FieldValueFactory * clone() const { return new FieldValueFactory(*this); }
private:
    DataType::CP _dataType;
};
}

FieldValue::IArray::UP
FieldValue::createArray(const DataType & baseType)
{
    switch(baseType.getId()) {
    case DataType::T_INT:
        return IArray::UP(new PrimitiveArrayT<IntFieldValue, FieldValue>());
    case DataType::T_FLOAT:
        return IArray::UP(new PrimitiveArrayT<FloatFieldValue, FieldValue>());
    case DataType::T_STRING:
        return IArray::UP(new PrimitiveArrayT<StringFieldValue, FieldValue>());
    case DataType::T_RAW:
        return IArray::UP(new PrimitiveArrayT<RawFieldValue, FieldValue>());
    case DataType::T_LONG:
        return IArray::UP(new PrimitiveArrayT<LongFieldValue, FieldValue>());
    case DataType::T_DOUBLE:
        return IArray::UP(new PrimitiveArrayT<DoubleFieldValue, FieldValue>());
    case DataType::T_BYTE:
        return IArray::UP(new PrimitiveArrayT<ByteFieldValue, FieldValue>());
    default:
        return IArray::UP(new ComplexArrayT<FieldValue>(FieldValueFactory::UP(new FieldValueFactory(DataType::UP(baseType.clone())))));
    }
}




} // document
