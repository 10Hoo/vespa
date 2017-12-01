// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "dense_dot_product_function.h"
#include "dense_xw_product_function.h"
#include "dense_tensor_function_compiler.h"
#include <vespa/eval/eval/operation.h>
#include <vespa/vespalib/test/insertion_operators.h>
#include <iostream>

using namespace vespalib::eval;
using namespace vespalib::eval::tensor_function;
using namespace vespalib::eval::operation;

namespace vespalib {
namespace tensor {

namespace {

bool is1dDenseTensor(const ValueType &type) {
    return (type.is_dense() && (type.dimensions().size() == 1));
}

bool isConcreteDenseTensor(const ValueType &type, size_t d) {
    return (type.is_dense() && (type.dimensions().size() == d) && !type.is_abstract());
}

bool isDenseDotProduct(const ValueType &res, const ValueType &lhsType, const ValueType &rhsType) {
    return (res.is_double() &&
            is1dDenseTensor(lhsType) &&
            is1dDenseTensor(rhsType) &&
            (lhsType.dimensions()[0].name == rhsType.dimensions()[0].name));
}

bool isDenseXWProduct(const ValueType &res, const ValueType &vec, const ValueType &mat) {
    if (isConcreteDenseTensor(res, 1) &&
        isConcreteDenseTensor(vec, 1) &&
        isConcreteDenseTensor(mat, 2))
    {
        size_t res_idx = mat.dimension_index(res.dimensions()[0].name);
        size_t vec_idx = mat.dimension_index(vec.dimensions()[0].name);
        size_t npos = ValueType::Dimension::npos;
        if ((res_idx != npos) && (vec_idx != npos) && (res_idx != vec_idx)) {
            return ((mat.dimensions()[res_idx].size == res.dimensions()[0].size) &&
                    (mat.dimensions()[vec_idx].size == vec.dimensions()[0].size));
        }
    }
    return false;
}

const TensorFunction &createDenseXWProduct(const ValueType &res, const Inject &vec, const Inject &mat, Stash &stash) {
    bool common_is_inner = (mat.result_type.dimension_index(vec.result_type.dimensions()[0].name) == 1);
    return stash.create<DenseXWProductFunction>(res, vec.tensor_id, mat.tensor_id,
                                                vec.result_type.dimensions()[0].size,
                                                res.dimensions()[0].size,
                                                common_is_inner);
}

struct InnerProductFunctionCompiler
{
    static const TensorFunction &compile(const Node &expr, Stash &stash) {
        const Reduce *reduce = as<Reduce>(expr);
        if (reduce && (reduce->aggr == Aggr::SUM)) {
            const Join *join = as<Join>(reduce->tensor);
            if (join && (join->function == Mul::f)) {
                const Inject *lhs = as<Inject>(join->lhs_tensor);
                const Inject *rhs = as<Inject>(join->rhs_tensor);
                if (lhs && rhs) {
                    if (isDenseDotProduct(expr.result_type, lhs->result_type, rhs->result_type)) {
                        return stash.create<DenseDotProductFunction>(lhs->tensor_id, rhs->tensor_id);
                    }
                    if (isDenseXWProduct(expr.result_type, lhs->result_type, rhs->result_type)) {
                        return createDenseXWProduct(expr.result_type, *lhs, *rhs, stash);
                    }
                    if (isDenseXWProduct(expr.result_type, rhs->result_type, lhs->result_type)) {
                        return createDenseXWProduct(expr.result_type, *rhs, *lhs, stash);
                    }
                }
            }
        }
        return expr;
    }
};

}

const TensorFunction &
DenseTensorFunctionCompiler::compile(const eval::tensor_function::Node &expr, Stash &stash)
{
    return InnerProductFunctionCompiler::compile(expr, stash);
}

} // namespace tensor
} // namespace vespalib
