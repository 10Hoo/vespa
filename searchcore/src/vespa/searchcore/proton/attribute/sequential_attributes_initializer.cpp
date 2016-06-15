// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "sequential_attributes_initializer.h"
#include "attributemanager.h"

using search::AttributeVector;
using search::SerialNum;

namespace proton {

SequentialAttributesInitializer::SequentialAttributesInitializer(uint32_t docIdLimit)
    : AttributesInitializerBase(),
      _docIdLimit(docIdLimit)
{
}

void
SequentialAttributesInitializer::add(AttributeInitializer::UP initializer)
{
    AttributeVector::SP attribute = initializer->init();
    if (attribute) {
        considerPadAttribute(*attribute, initializer->getCurrentSerialNum(), _docIdLimit);
        _initializedAttributes.push_back(attribute);
    }
}

} // namespace proton
