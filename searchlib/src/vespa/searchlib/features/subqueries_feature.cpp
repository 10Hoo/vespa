// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".features.subqueries_feature");
#include "subqueries_feature.h"
#include "utils.h"

using namespace search::fef;

namespace search {
namespace features {

SubqueriesExecutor::SubqueriesExecutor(const IQueryEnvironment &env,
                                       uint32_t fieldId)
    : FeatureExecutor(),
      _handles(),
      _md(nullptr)
{
    for (uint32_t i = 0; i < env.getNumTerms(); ++i) {
        TermFieldHandle handle = util::getTermFieldHandle(env, i, fieldId);
        if (handle != IllegalHandle) {
            _handles.push_back(handle);
        }
    }
}

void SubqueriesExecutor::execute(uint32_t docId) {
    uint32_t lsb = 0;
    uint32_t msb = 0;
    for (uint32_t i = 0; i < _handles.size(); ++i) {
        const TermFieldMatchData *tfmd = _md->resolveTermField(_handles[i]);
        if (tfmd->getDocId() == docId) {
            lsb |= static_cast<uint32_t>(tfmd->getSubqueries());
            msb |= tfmd->getSubqueries() >> 32;
        }
    }
    outputs().set_number(0, lsb);
    outputs().set_number(1, msb);
}

void
SubqueriesExecutor::handle_bind_match_data(fef::MatchData &md)
{
    _md = &md;
}

//-----------------------------------------------------------------------------

bool SubqueriesBlueprint::setup(const IIndexEnvironment &,
                                const ParameterList &params) {
    _field = params[0].asField();
    describeOutput("lsb", "32 least significant bits of the subquery bitmap"
                   " for the given field");
    describeOutput("msb", "32 most significant bits of the subquery bitmap"
                   " for the given field");
    return true;
}

FeatureExecutor &
SubqueriesBlueprint::createExecutor(const IQueryEnvironment &queryEnv, vespalib::Stash &stash) const {
    return stash.create<SubqueriesExecutor>(queryEnv, _field->id());
}

} // namespace features
} // namespace search
