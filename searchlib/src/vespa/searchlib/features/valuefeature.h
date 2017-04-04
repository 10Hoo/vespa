// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <string>
#include <vector>
#include <vespa/searchlib/fef/blueprint.h>
#include <vespa/searchlib/fef/featureexecutor.h>
#include <vespa/searchlib/common/feature.h>

namespace search {
namespace features {

class ValueExecutor : public fef::FeatureExecutor
{
private:
    std::vector<feature_t> _values;

public:
    ValueExecutor(const std::vector<feature_t> & values);
    bool isPure() override { return true; }
    void execute(uint32_t docId) override;
    const std::vector<feature_t> & getValues() const { return _values; }
};

class SingleZeroValueExecutor : public fef::FeatureExecutor
{
public:
    SingleZeroValueExecutor() : FeatureExecutor() {}
    bool isPure() override { return true; }
    void execute(uint32_t docId) override;
};


class ValueBlueprint : public fef::Blueprint
{
private:
    std::vector<feature_t> _values;

public:
    ValueBlueprint();

    void visitDumpFeatures(const fef::IIndexEnvironment & indexEnv,
                           fef::IDumpFeatureVisitor & visitor) const override;
    fef::Blueprint::UP createInstance() const override { return Blueprint::UP(new ValueBlueprint()); }
    fef::ParameterDescriptions getDescriptions() const override {
        return fef::ParameterDescriptions().desc().number().number().repeat();
    }
    bool setup(const fef::IIndexEnvironment & env,
               const fef::ParameterList & params) override;
    fef::FeatureExecutor &createExecutor(const fef::IQueryEnvironment &queryEnv, vespalib::Stash &stash) const override;
};

} // namespace features
} // namespace search

