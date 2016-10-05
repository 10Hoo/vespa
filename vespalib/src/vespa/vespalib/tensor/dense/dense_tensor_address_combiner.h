// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/vespalib/tensor/dense/dense_tensor.h>

namespace vespalib {
namespace tensor {

/**
 * Combines two dense tensor addresses to a new tensor address.
 * The resulting dimensions is the union of the input dimensions and
 * common dimensions must have matching labels.
 */
class DenseTensorAddressCombiner
{
public:
    using Address = std::vector<size_t>;
    using DimensionsMeta = DenseTensor::DimensionsMeta;

private:
    enum class AddressOp {
        LHS,
        RHS,
        BOTH
    };

    using CellsIterator = DenseTensor::CellsIterator;

    std::vector<AddressOp> _ops;
    Address _combinedAddress;

public:
    DenseTensorAddressCombiner(const DimensionsMeta &lhs,
                               const DimensionsMeta &rhs);

    bool combine(const CellsIterator &lhsItr,
                 const CellsIterator &rhsItr);
    const Address &address() const { return _combinedAddress; }

    static DimensionsMeta combineDimensions(const DimensionsMeta &lhs, const DimensionsMeta &rhs);

};

} // namespace vespalib::tensor
} // namespace vespalib
