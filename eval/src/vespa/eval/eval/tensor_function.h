// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <memory>
#include <vector>
#include <vespa/vespalib/stllike/string.h>
#include <vespa/vespalib/util/arrayref.h>
#include "value_type.h"
#include "value.h"
#include "aggr.h"

namespace vespalib {

class Stash;

namespace eval {

class Tensor;

//-----------------------------------------------------------------------------

/**
 * A tensor function that can be evaluated. A TensorFunction will
 * typically be produced by an implementation-specific compile step
 * that takes an implementation-independent intermediate
 * representation of the tensor function as input (tree of
 * tensor_function::Node objects).
 **/
struct TensorFunction
{
    /**
     * Evaluate this tensor function based on the given
     * parameters. The given stash can be used to store temporary
     * objects that need to be kept alive for the return value to be
     * valid. The return value must conform to the result type
     * indicated by the intermediate representation describing this
     * tensor function.
     *
     * @return result of evaluating this tensor function
     * @param params external values needed to evaluate this function
     * @param stash heterogeneous object store
     **/
    virtual const Value &eval(ConstArrayRef<Value::CREF> params, Stash &stash) const = 0;
    virtual ~TensorFunction() {}
};

//-----------------------------------------------------------------------------

struct TensorFunctionVisitor;

namespace tensor_function {

using map_fun_t = double (*)(double);
using join_fun_t = double (*)(double, double);

/**
 * Interface used to describe a tensor function as a tree of nodes
 * with information about operation sequencing and intermediate result
 * types. Each node in the tree will describe a single tensor
 * operation. This is the intermediate representation of a tensor
 * function.
 *
 * The intermediate representation of a tensor function can also be
 * used to evaluate the tensor function it represents directly. This
 * will invoke the immediate API on the tensor engine associated with
 * the input tensors. In other words, the intermediate representation
 * 'compiles to itself'.
 **/
struct Node : public TensorFunction
{
    const ValueType result_type;
    Node(const ValueType &result_type_in) : result_type(result_type_in) {}
    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node &&) = delete;
};

/**
 * Simple typecasting utility.
 */
template <typename T>
const T *as(const Node &node) { return dynamic_cast<const T *>(&node); }

struct Inject : Node {
    const size_t tensor_id;
    Inject(const ValueType &result_type_in,
           size_t tensor_id_in)
        : Node(result_type_in), tensor_id(tensor_id_in) {}
    const Value &eval(ConstArrayRef<Value::CREF> params, Stash &) const override;
};

struct Reduce : Node {
    const Node &tensor;
    const Aggr aggr;
    const std::vector<vespalib::string> dimensions;
    Reduce(const ValueType &result_type_in,
           const Node &tensor_in,
           Aggr aggr_in,
           const std::vector<vespalib::string> &dimensions_in)
        : Node(result_type_in), tensor(tensor_in), aggr(aggr_in), dimensions(dimensions_in) {}
    const Value &eval(ConstArrayRef<Value::CREF> params, Stash &stash) const override;
};

struct Map : Node {
    const Node &tensor;
    const map_fun_t function;    
    Map(const ValueType &result_type_in,
        const Node &tensor_in,
        map_fun_t function_in)
        : Node(result_type_in), tensor(tensor_in), function(function_in) {}
    const Value &eval(ConstArrayRef<Value::CREF> params, Stash &stash) const override;
};

struct Join : Node {
    const Node &lhs_tensor;
    const Node &rhs_tensor;
    const join_fun_t function;    
    Join(const ValueType &result_type_in,
         const Node &lhs_tensor_in,
         const Node &rhs_tensor_in,
         join_fun_t function_in)
        : Node(result_type_in), lhs_tensor(lhs_tensor_in),
          rhs_tensor(rhs_tensor_in), function(function_in) {}
    const Value &eval(ConstArrayRef<Value::CREF> params, Stash &stash) const override;
};

const Node &inject(const ValueType &type, size_t tensor_id, Stash &stash);
const Node &reduce(const Node &tensor, Aggr aggr, const std::vector<vespalib::string> &dimensions, Stash &stash);
const Node &map(const Node &tensor, map_fun_t function, Stash &stash);
const Node &join(const Node &lhs_tensor, const Node &rhs_tensor, join_fun_t function, Stash &stash);

} // namespace vespalib::eval::tensor_function
} // namespace vespalib::eval
} // namespace vespalib
