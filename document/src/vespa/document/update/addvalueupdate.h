// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * @class document::AddValueUpdate
 * @ingroup document
 *
 * @brief Represents an update that specifies an addition to a field value.
 */
#pragma once

#include <vespa/document/fieldvalue/fieldvalue.h>
#include <vespa/document/update/valueupdate.h>

namespace document {

class AddValueUpdate : public ValueUpdate {
    FieldValue::CP _value; // The field value to add by this update.
    int _weight; // The weight to assign to the contained value.

    // Used by ValueUpdate's static factory function
    // Private because it generates an invalid object.
    friend class ValueUpdate;
    AddValueUpdate() : ValueUpdate(), _value(0), _weight(1) {}
    ACCEPT_UPDATE_VISITOR;
public:
    typedef std::unique_ptr<AddValueUpdate> UP;

    /**
     * The default constructor requires initial values for all member variables.
     *
     * @param value The field value to add.
     * @param weight The weight for the field value.
     */
    AddValueUpdate(const FieldValue& value, int weight = 1)
        : ValueUpdate(),
          _value(value.clone()),
          _weight(weight) {}

    virtual bool operator==(const ValueUpdate& other) const;

    /** @return the field value to add during this update. */
    const FieldValue& getValue() const { return *_value; }

    /** @return The weight to assign to the value of this. */
    int getWeight() const { return _weight; }

    /**
     * Sets the field value to add during this update.
     *
     * @param value The new field value.
     * @return A reference to this object so you can chain calls.
     */
    AddValueUpdate& setValue(const FieldValue& value) {
        _value.reset(value.clone());
        return *this;
    }

    /**
     * Sets the weight to assign to the value of this.
     *
     * @return A reference to this object so you can chain calls.
     */
    AddValueUpdate& setWeight(int weight) {
        _weight = weight;
        return *this;
    }

    // ValueUpdate implementation
    virtual void checkCompatibility(const Field& field) const;
    virtual bool applyTo(FieldValue& value) const;
    virtual void printXml(XmlOutputStream& xos) const;
    virtual void print(std::ostream& out, bool verbose,
                       const std::string& indent) const;
    virtual void deserialize(const DocumentTypeRepo& repo,
                             const DataType& type,
                             ByteBuffer& buffer, uint16_t version);
    virtual AddValueUpdate* clone() const { return new AddValueUpdate(*this); }

    DECLARE_IDENTIFIABLE(AddValueUpdate);

};

} // document

