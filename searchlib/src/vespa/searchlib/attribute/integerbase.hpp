// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "integerbase.hpp"

namespace search {

template<typename T>
uint32_t
IntegerAttributeTemplate<T>::getRawValues(DocId, const multivalue::Value<T> * &) const {
    throw std::runtime_error(getNativeClassName() + "::getRawValues() not implemented.");
}

template<typename T>
bool
IntegerAttributeTemplate<T>::findEnum(const char *value, EnumHandle &e) const {
    vespalib::asciistream iss(value);
    int64_t ivalue = 0;
    try {
        iss >> ivalue;
    } catch (const vespalib::IllegalArgumentException &) {
    }
    return findEnum(ivalue, e);
}

template<typename T>
largeint_t
IntegerAttributeTemplate<T>::getIntFromEnum(EnumHandle e) const {
    T v(getFromEnum(e));
    if (attribute::isUndefined<T>(v)) {
        return attribute::getUndefined<largeint_t>();
    }
    return v;
}

template<typename T>
long
IntegerAttributeTemplate<T>::onSerializeForAscendingSort(DocId doc, void * serTo, long available, const common::BlobConverter * bc) const {
    (void) bc;
    if (available >= long(sizeof(T))) {
        T origValue(get(doc));
        vespalib::serializeForSort< vespalib::convertForSort<T, true> >(origValue, serTo);
    } else {
        return -1;
    }
    return sizeof(T);
}

template<typename T>
long
IntegerAttributeTemplate<T>::onSerializeForDescendingSort(DocId doc, void * serTo, long available, const common::BlobConverter * bc) const {
    (void) bc;
    if (available >= long(sizeof(T))) {
        T origValue(get(doc));
        vespalib::serializeForSort< vespalib::convertForSort<T, false> >(origValue, serTo);
    } else {
        return -1;
    }
    return sizeof(T);
}

}

