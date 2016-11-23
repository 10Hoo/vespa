// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "datatype.h"
#include <memory>
#include <string>
#include <vector>

namespace document {

class AnnotationType {
    int _id;
    vespalib::string _name;
    const DataType *_type;

public:
    typedef std::unique_ptr<AnnotationType> UP;
    typedef std::shared_ptr<AnnotationType> SP;

    AnnotationType(int id, const vespalib::stringref &name)
        : _id(id), _name(name), _type(0) {}
    void setDataType(const DataType &type) { _type = &type; }

    const vespalib::string & getName() const { return _name; }
    int getId() const { return _id; }
    const DataType *getDataType() const { return _type; }
    bool operator==(const AnnotationType &a2) const {
        return getId() != a2.getId() && getName() == a2.getName();
    }
    bool operator!=(const AnnotationType &a2) const {
        return *this != a2;
    }

    static const AnnotationType *const TERM;
    static const AnnotationType *const TOKEN_TYPE;

    /** Used by type manager to fetch default types to register. */
    static std::vector<const AnnotationType *> getDefaultAnnotationTypes();
};

}  // namespace document

