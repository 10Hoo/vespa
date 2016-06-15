// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>

#include <vespa/document/base/exceptions.h>
#include <vespa/document/datatype/arraydatatype.h>
#include <vespa/document/datatype/mapdatatype.h>
#include <vespa/document/fieldvalue/arrayfieldvalue.h>
#include <vespa/document/fieldvalue/weightedsetfieldvalue.h>
#include <vespa/document/util/bytebuffer.h>
#include <vespa/vespalib/objects/identifiable.h>

using vespalib::Identifiable;

LOG_SETUP(".document.fieldvalue.weightedset");

/// \todo TODO (was warning):  Find a way to search through internal map without duplicating keys to create shared pointers.

namespace document {

IMPLEMENT_IDENTIFIABLE_ABSTRACT(WeightedSetFieldValue, CollectionFieldValue);

namespace {
const DataType &getKeyType(const DataType &type) {
    const WeightedSetDataType *wtype =
        Identifiable::cast<const WeightedSetDataType *>(&type);
    if (!wtype) {
        throw vespalib::IllegalArgumentException(
                "Cannot generate a weighted set value with non-weighted set "
                "type " + type.toString() + ".", VESPA_STRLOC);
    }
    return wtype->getNestedType();
}
}  // namespace

WeightedSetFieldValue::WeightedSetFieldValue(const DataType &type)
    : CollectionFieldValue(type),
      _map_type(new MapDataType(getKeyType(type), *DataType::INT)),
      _map(*_map_type),
      _altered(true) {
}

WeightedSetFieldValue::~WeightedSetFieldValue()
{
}

void WeightedSetFieldValue::verifyKey(const FieldValue & v)
{
    if (!getNestedType().isValueType(v)) {
        throw InvalidDataTypeException(*v.getDataType(), getNestedType(),
                                       VESPA_STRLOC);
    }
}

bool
WeightedSetFieldValue::add(const FieldValue& key, int weight)
{
    verifyKey(key);
    const WeightedSetDataType & wdt(static_cast<const WeightedSetDataType&>(*_type));
    _altered = true;
    if (wdt.removeIfZero() && (weight == 0)) {
        _map.erase(key);
        return false;
    }
    return _map.insert(FieldValue::UP(key.clone()), FieldValue::UP(new IntFieldValue(weight)));
}

bool
WeightedSetFieldValue::addIgnoreZeroWeight(const FieldValue& key,
                                           int32_t weight)
{
    verifyKey(key);
    _altered = true;
    return _map.insert(FieldValue::UP(key.clone()),
                       FieldValue::UP(new IntFieldValue(weight)));
}

void
WeightedSetFieldValue::push_back(FieldValue::UP key, int weight)
{
    _altered = true;
    _map.push_back(std::move(key), FieldValue::UP(new IntFieldValue(weight)));
}

void
WeightedSetFieldValue::increment(const FieldValue& key, int val)
{
    verifyKey(key);
    WeightedFieldValueMap::iterator it(_map.find(key));
    const WeightedSetDataType & wdt(static_cast<const WeightedSetDataType&>(*_type));
    if (wdt.createIfNonExistent()) {
        if (it == _map.end()) {
            _map.insert(FieldValue::UP(key.clone()), FieldValue::UP(new IntFieldValue(val)));
        } else {
            IntFieldValue& fv = static_cast<IntFieldValue&>(*it->second);
            fv.setValue(fv.getValue() + val);
            if (wdt.removeIfZero() && fv.getValue() == 0) {
                _map.erase(key);
            }
        }
    } else {
        if (it == _map.end()) {
           throw vespalib::IllegalStateException("Cannot modify non-existing "
                    "entry in weightedset without createIfNonExistent set",
                    VESPA_STRLOC);
        }
        IntFieldValue& fv = static_cast<IntFieldValue&>(*it->second);
        fv.setValue(fv.getValue() + val);
        if (wdt.removeIfZero() && fv.getValue() == 0) {
            _map.erase(key);
        }
    }
    _altered = true;
}

int32_t
WeightedSetFieldValue::get(const FieldValue& key, int32_t defaultValue) const
{
    WeightedFieldValueMap::const_iterator it = find(key);
    return (it == end()
            ? defaultValue
            : static_cast<const IntFieldValue&>(*it->second).getValue());
}

bool
WeightedSetFieldValue::containsValue(const FieldValue& key) const
{
    return _map.contains(key);
}

bool
WeightedSetFieldValue::removeValue(const FieldValue& key)
{
    bool result = _map.erase(key);
    _altered |= result;
    return result;
}

FieldValue&
WeightedSetFieldValue::assign(const FieldValue& value)
{
    if (getDataType()->isValueType(value)) {
        return operator=(static_cast<const WeightedSetFieldValue&>(value));
    }
    return FieldValue::assign(value);
}

int
WeightedSetFieldValue::compare(const FieldValue& other) const
{
    int diff = CollectionFieldValue::compare(other);
    if (diff != 0) return diff;

    const WeightedSetFieldValue& wset(
            dynamic_cast<const WeightedSetFieldValue&>(other));
    return _map.compare(wset._map);
}

void
WeightedSetFieldValue::printXml(XmlOutputStream& xos) const
{
    for (WeightedFieldValueMap::const_iterator it = _map.begin();
         it != _map.end(); ++it)
    {
        const IntFieldValue& fv = static_cast<const IntFieldValue&>(*it->second);
        xos << XmlTag("item") << XmlAttribute("weight", fv.getValue())
            << *it->first
            << XmlEndTag();
    }
}

void
WeightedSetFieldValue::print(std::ostream& out, bool verbose,
                       const std::string& indent) const
{
    out << getDataType()->getName() << "(";

    int count = 0;
    for (WeightedFieldValueMap::const_iterator it = _map.begin();
         it != _map.end(); ++it)
    {
        if (count++ != 0) {
            out << ",";
        }
        out << "\n" << indent << "  ";
        it->first->print(out, verbose, indent + "  ");
        const IntFieldValue& fv = static_cast<const IntFieldValue&>(*it->second);
        out << " - weight " << fv.getValue();
    }
    if (_map.size() > 0) out << "\n" << indent;
    out << ")";
}

bool
WeightedSetFieldValue::hasChanged() const
{
    // Keys are not allowed to change in a map, so the keys should not be
    // referred to externally, and should thus not need to be checked.
    return _altered;
}

WeightedSetFieldValue::const_iterator
WeightedSetFieldValue::find(const FieldValue& key) const
{
    return _map.find(key);
}

WeightedSetFieldValue::iterator
WeightedSetFieldValue::find(const FieldValue& key)
{
    return _map.find(key);
}

FieldValue::IteratorHandler::ModificationStatus
WeightedSetFieldValue::onIterateNested(FieldPath::const_iterator start,
                                       FieldPath::const_iterator end_,
                                       IteratorHandler & handler) const
{
    LOG(spam, "iterating over WeightedSetFieldValue");
    return _map.iterateNestedImpl(start, end_, handler, *this);
}

} // document
