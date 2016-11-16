// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "attributevector.h"

namespace search {
namespace attribute {

/*
 * Function for loading mapping from document id to array of enum indexes
 * or values from enumerated attribute reader.
 */
template <class MvMapping, class Saver>
uint32_t
loadFromEnumeratedMultiValue(MvMapping &mapping,
                             AttributeVector::ReaderBase &attrReader,
                             vespalib::ConstArrayRef<typename MvMapping::MultiValueType::ValueType> enumValueToValueMap,
                             Saver saver) __attribute((noinline));

/*
 * Function for loading mapping from document id to of enum index or
 * value from enumerated attribute reader.
 */
template <class Vector, class Saver>
void
loadFromEnumeratedSingleValue(Vector &vector,
                              vespalib::GenerationHolder &genHolder,
                              AttributeVector::ReaderBase &attrReader,
                              vespalib::ConstArrayRef<typename Vector::ValueType> enumValueToValueMap,
                              Saver saver) __attribute((noinline));

} // namespace search::attribute
} // namespace search
