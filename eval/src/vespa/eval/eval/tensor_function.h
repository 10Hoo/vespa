// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <memory>
#include <vector>
#include <vespa/vespalib/stllike/string.h>
#include <vespa/vespalib/util/arrayref.h>
#include "lazy_params.h"
#include "value_type.h"
#include "value.h"
#include "aggr.h"

namespace vespalib {

class Stash;

namespace eval {

class Tensor;

//-----------------------------------------------------------------------------

/**
 * Interface used to describe a tensor function as a tree of nodes
 * with information about operation sequencing and intermediate result
 * types. Each node in the tree describes a single tensor
 * operation. This is the intermediate representation of a tensor
 * function.
 *
 * A tensor function will initially be created based on a Function
 * (expression AST) and associated type-resolving. In this tree, each
 * node will directly represent a single call to the tensor engine
 * immediate API.
 *
 * The generic implementation-independent tree will then be optimized
 * (in-place, bottom-up) where sub-expressions may be replaced with
 * optimized implementation-specific alternatives.
 *
 * This leaves us with a mixed-mode tree with some generic and some
 * specialized nodes, that may be evaluated recursively.
 **/
struct TensorFunction
{
    TensorFunction(const TensorFunction &) = delete;
    TensorFunction &operator=(const TensorFunction &) = delete;
    TensorFunction(TensorFunction &&) = delete;
    TensorFunction &operator=(TensorFunction &&) = delete;
    TensorFunction() {}

    /**
     * Reference to a sub-tree. References are replaceable to enable
     * in-place bottom-up optimization.
     **/
    class Child {
    private:
        mutable const TensorFunction *ptr;
    public:
        using CREF = std::reference_wrapper<const Child>;
        Child(const TensorFunction &child) : ptr(&child) {}
        const TensorFunction &get() const { return *ptr; }
        void set(const TensorFunction &child) const { ptr = &child; }
    };
    virtual const ValueType &result_type() const = 0;

    /**
     * Push references to all children (NB: implementation must use
     * Child class for all sub-expression references) on the given
     * vector. This is needed to enable optimization of trees where
     * the core algorithm does not need to know concrete node types.
     *
     * @params children where to put your children references
     **/
    virtual void push_children(std::vector<Child::CREF> &children) const = 0;

    /**
     * Evaluate this tensor function based on the given
     * parameters. The given stash can be used to store temporary
     * objects that need to be kept alive for the return value to be
     * valid. The return value must conform to 'result_type'.
     *
     * @return result of evaluating this tensor function
     * @param params external values needed to evaluate this function
     * @param stash heterogeneous object store
     **/
    virtual const Value &eval(const LazyParams &params, Stash &stash) const = 0;
    virtual ~TensorFunction() {}
};

/**
 * Simple typecasting utility.
 */
template <typename T>
const T *as(const TensorFunction &node) { return dynamic_cast<const T *>(&node); }

//-----------------------------------------------------------------------------

namespace tensor_function {

using map_fun_t = double (*)(double);
using join_fun_t = double (*)(double, double);

class Node : public TensorFunction
{
private:
    ValueType _result_type;
public:
    Node(const ValueType &result_type_in) : _result_type(result_type_in) {}
    const ValueType &result_type() const override { return _result_type; }
};

class Inject : public Node
{
private:
    size_t _param_idx;
public:
    Inject(const ValueType &result_type_in,
           size_t param_idx_in)
        : Node(result_type_in), _param_idx(param_idx_in) {}
    size_t param_idx() const { return _param_idx; }
    void push_children(std::vector<Child::CREF> &children) const override;
    const Value &eval(const LazyParams &params, Stash &) const override;
};

class Reduce : public Node
{
private:
    Child _child;
    Aggr _aggr;
    std::vector<vespalib::string> _dimensions;
public:
    Reduce(const ValueType &result_type_in,
           const TensorFunction &child_in,
           Aggr aggr_in,
           const std::vector<vespalib::string> &dimensions_in)
        : Node(result_type_in), _child(child_in), _aggr(aggr_in), _dimensions(dimensions_in) {}
    const TensorFunction &child() const { return _child.get(); }
    Aggr aggr() const { return _aggr; }
    const std::vector<vespalib::string> dimensions() const { return _dimensions; }
    void push_children(std::vector<Child::CREF> &children) const override;
    const Value &eval(const LazyParams &params, Stash &stash) const override;
};

class Map : public Node
{
private:
    Child _child;
    map_fun_t _function;
public:
    Map(const ValueType &result_type_in,
        const TensorFunction &child_in,
        map_fun_t function_in)
        : Node(result_type_in), _child(child_in), _function(function_in) {}
    const TensorFunction &child() const { return _child.get(); }
    map_fun_t function() const { return _function; }
    void push_children(std::vector<Child::CREF> &children) const override;
    const Value &eval(const LazyParams &params, Stash &stash) const override;
};

class Join : public Node
{
private:
    Child _lhs;
    Child _rhs;
    join_fun_t _function;    
public:
    Join(const ValueType &result_type_in,
         const TensorFunction &lhs_in,
         const TensorFunction &rhs_in,
         join_fun_t function_in)
        : Node(result_type_in), _lhs(lhs_in),
          _rhs(rhs_in), _function(function_in) {}
    const TensorFunction &lhs() const { return _lhs.get(); }
    const TensorFunction &rhs() const { return _rhs.get(); }
    join_fun_t function() const { return _function; }
    void push_children(std::vector<Child::CREF> &children) const override;
    const Value &eval(const LazyParams &params, Stash &stash) const override;
};

const Node &inject(const ValueType &type, size_t param_idx, Stash &stash);
const Node &reduce(const Node &child, Aggr aggr, const std::vector<vespalib::string> &dimensions, Stash &stash);
const Node &map(const Node &child, map_fun_t function, Stash &stash);
const Node &join(const Node &lhs, const Node &rhs, join_fun_t function, Stash &stash);

} // namespace vespalib::eval::tensor_function
} // namespace vespalib::eval
} // namespace vespalib
