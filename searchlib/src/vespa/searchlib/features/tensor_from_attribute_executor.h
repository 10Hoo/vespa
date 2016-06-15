// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchcommon/attribute/iattributevector.h>
#include <vespa/searchlib/fef/featureexecutor.h>
#include <vespa/vespalib/eval/value.h>
#include <vespa/vespalib/stllike/string.h>
#include <vespa/vespalib/tensor/default_tensor.h>

namespace search {
namespace features {

/**
 * Feature executor that extracts the content from an attribute vector
 * and converts that into a tensor.
 */
template <typename WeightedBufferType>
class TensorFromAttributeExecutor : public fef::FeatureExecutor
{
private:
    const search::attribute::IAttributeVector *_attribute;
    vespalib::string _dimension;
    WeightedBufferType _attrBuffer;
    vespalib::eval::TensorValue::UP _tensor;

public:
    TensorFromAttributeExecutor(const search::attribute::IAttributeVector *attribute,
                                const vespalib::string &dimension)
        : _attribute(attribute),
          _dimension(dimension),
          _attrBuffer(),
          _tensor()
    {
        _attrBuffer.allocate(_attribute->getMaxValueCount());
    }
    virtual void execute(fef::MatchData &data);
};

template <typename WeightedBufferType>
void
TensorFromAttributeExecutor<WeightedBufferType>::execute(fef::MatchData &data)
{
    _attrBuffer.fill(*_attribute, data.getDocId());
    vespalib::tensor::DefaultTensor::builder builder;
    vespalib::tensor::TensorBuilder::Dimension dimensionEnum = builder.define_dimension(_dimension);
    for (size_t i = 0; i < _attrBuffer.size(); ++i) {
        builder.add_label(dimensionEnum, vespalib::string(_attrBuffer[i].value()));
        builder.add_cell(_attrBuffer[i].weight());
    }
    _tensor = vespalib::eval::TensorValue::UP(new vespalib::eval::TensorValue(builder.build()));
    *data.resolve_object_feature(outputs()[0]) = *_tensor;
}

} // namespace features
} // namespace search
