// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "multivalueattributesaver.h"

namespace search {

/*
 * Class for saving a multivalue attribute.
 *
 * Template argument MultiValueT is either  multivalue::Value<ValueType> or
 * multivalue::WeightedValue<ValueType>
 */
template <typename MultiValueT>
class MultiValueNumericAttributeSaver : public MultiValueAttributeSaver
{
    using Parent = MultiValueAttributeSaver;
    using MultiValueType = MultiValueT;
    using ValueType = typename MultiValueType::ValueType;
    using GenerationHandler = vespalib::GenerationHandler;
    using Parent::_frozenIndices;
    using MultiValueMapping = attribute::MultiValueMapping2<MultiValueType>;

    const MultiValueMapping &_mvMapping;
public:
    virtual bool onSave(IAttributeSaveTarget &saveTarget) override;
    MultiValueNumericAttributeSaver(GenerationHandler::Guard &&guard,
                                    const IAttributeSaveTarget::Config &cfg,
                                    const MultiValueMapping &mvMapping);

    virtual ~MultiValueNumericAttributeSaver();
};


} // namespace search
