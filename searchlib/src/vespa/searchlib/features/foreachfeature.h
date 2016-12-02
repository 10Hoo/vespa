// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchlib/fef/blueprint.h>
#include <vespa/searchlib/fef/featureexecutor.h>
#include <limits>


namespace search {
namespace features {

/**
 * Implements the executor for the foreach feature.
 * Uses a condition and operation template class to perform the computation.
 */
template <typename CO, typename OP>
class ForeachExecutor : public search::fef::FeatureExecutor {
private:
    CO        _condition;
    OP        _operation;
    uint32_t  _numInputs;

public:
    ForeachExecutor(const CO & condition, uint32_t numInputs);
    virtual void execute(search::fef::MatchData & data);
};


/**
 * Base class for condition template class.
 **/
class ConditionBase {
protected:
    feature_t _param;
public:
    ConditionBase(feature_t param = 0) : _param(param) {}
};

/**
 * Implements the true condition.
 **/
struct TrueCondition : public ConditionBase {
    bool useValue(feature_t val) { (void) val; return true; }
};

/**
 * Implements the less than condition.
 **/
struct LessThanCondition : public ConditionBase {
    LessThanCondition(feature_t param) : ConditionBase(param) {}
    bool useValue(feature_t val) { return val < _param; }
};

/**
 * Implements the greater than condition.
 **/
struct GreaterThanCondition : public ConditionBase {
    GreaterThanCondition(feature_t param) : ConditionBase(param) {}
    bool useValue(feature_t val) { return val > _param; }
};


/**
 * Base class for operation template class.
 */
class OperationBase {
protected:
    feature_t _result;
public:
    OperationBase() : _result(0) {}
    feature_t getResult() const { return _result; }
};

/**
 * Implements sum operation.
 **/
struct SumOperation : public OperationBase {
    void reset() { _result = 0; }
    void onValue(feature_t val) { _result += val; }
};

/**
 * Implements product operation.
 **/
struct ProductOperation : public OperationBase {
    void reset() { _result = 1; }
    void onValue(feature_t val) { _result *= val; }
};

/**
 * Implements average operation.
 **/
class AverageOperation : public OperationBase {
private:
    uint32_t _numValues;
public:
    AverageOperation() : OperationBase(), _numValues(0) {}
    void reset() { _result = 0; _numValues = 0; }
    void onValue(feature_t val) { _result += val; ++_numValues; }
    feature_t getResult() const { return _numValues != 0 ? _result / _numValues : 0; }
};

/**
 * Implements max operation.
 **/
struct MaxOperation : public OperationBase {
    void reset() { _result = -std::numeric_limits<feature_t>::max(); }
    void onValue(feature_t val) { _result = std::max(val, _result); }
};

/**
 * Implements min operation.
 **/
struct MinOperation : public OperationBase {
    void reset() { _result = std::numeric_limits<feature_t>::max(); }
    void onValue(feature_t val) { _result = std::min(val, _result); }
};

/**
 * Implements count operation.
 **/
struct CountOperation : public OperationBase {
    void reset() { _result = 0; }
    void onValue(feature_t val) { (void) val; _result += 1; }
};


/**
 * Implements the blueprint for the foreach executor.
 */
class ForeachBlueprint : public search::fef::Blueprint {
private:
    enum Dimension {
        TERMS,
        FIELDS,
        ATTRIBUTES,
        ILLEGAL
    };
    struct ExecutorCreatorBase {
        virtual search::fef::FeatureExecutor::LP create(uint32_t numInputs) const = 0;
        virtual ~ExecutorCreatorBase() {}
    };

    Dimension _dimension;
    std::unique_ptr<ExecutorCreatorBase> _executorCreator;
    size_t _num_inputs;

    bool decideDimension(const vespalib::string & param);
    bool decideCondition(const vespalib::string & condition, const vespalib::string & operation);
    template <typename CO>
    bool decideOperation(CO condition, const vespalib::string & operation);
    template <typename CO, typename OP>
    void setExecutorCreator(CO condition);

public:
    /**
     * Constructs a blueprint.
     */
    ForeachBlueprint();

    // Inherit doc from Blueprint.
    virtual void visitDumpFeatures(const search::fef::IIndexEnvironment & env,
                                   search::fef::IDumpFeatureVisitor & visitor) const;

    // Inherit doc from Blueprint.
    virtual search::fef::Blueprint::UP createInstance() const;

    // Inherit doc from Blueprint.
    virtual search::fef::ParameterDescriptions getDescriptions() const {
        return search::fef::ParameterDescriptions().desc().string().string().feature().string().string();
    }

    // Inherit doc from Blueprint.
    virtual bool setup(const search::fef::IIndexEnvironment & env,
                       const search::fef::ParameterList & params);

    // Inherit doc from Blueprint.
    virtual search::fef::FeatureExecutor::LP createExecutor(const search::fef::IQueryEnvironment & env) const override;
};


} // namespace features
} // namespace search

