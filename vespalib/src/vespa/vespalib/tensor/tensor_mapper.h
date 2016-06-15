// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "tensor_type.h"

namespace vespalib {
namespace tensor {

class Tensor;

/**
 * Class to map a tensor to a given tensor type.  Dimensions in input
 * tensor not present in tensor type are ignored. Dimensions in tensor
 * type not present in input tensor gets default label (undefined
 * (empty string) for sparse tensors, 0 for dense tensors). Values are
 * accumulated for identical mapped addresses.
 *
 * Dense tensor type has further restrictions: label must contain only
 * numerical digits (0-9). Empty string equals 0.  If the label is
 * parsed to a value outside the dimension range or the parsing fails,
 * then the cell ((address, value) pair) is ignored.
 */
class TensorMapper
{
    TensorType _type;
public:
    TensorMapper(const TensorType &type);
    ~TensorMapper();

    template <typename TensorT>
    static std::unique_ptr<Tensor>
    mapToSparse(const Tensor &tensor, const TensorType &type);

    static std::unique_ptr<Tensor>
    mapToDense(const Tensor &tensor, const TensorType &type);

    std::unique_ptr<Tensor> map(const Tensor &tensor);
};


} // namespace vespalib::tensor
} // namespace vespalib
