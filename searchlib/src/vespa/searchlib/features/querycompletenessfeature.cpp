// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".features.querycompleteness");

#include <vespa/searchlib/fef/featurenamebuilder.h>
#include <vespa/searchlib/fef/fieldinfo.h>
#include <vespa/searchlib/fef/itermdata.h>
#include "querycompletenessfeature.h"
#include "utils.h"
#include <limits>

namespace search {
namespace features {

QueryCompletenessConfig::QueryCompletenessConfig() :
    fieldId(search::fef::IllegalHandle),
    fieldBegin(0),
    fieldEnd(std::numeric_limits<uint32_t>::max())
{
    // empty
}

QueryCompletenessExecutor::QueryCompletenessExecutor(const search::fef::IQueryEnvironment &env,
                                                     const QueryCompletenessConfig &config) :
    search::fef::FeatureExecutor(),
    _config(config),
    _fieldHandles()
{
    for (uint32_t i = 0; i < env.getNumTerms(); ++i) {
        const search::fef::TermFieldHandle handle = util::getTermFieldHandle(env, i, config.fieldId);
        if (handle != search::fef::IllegalHandle) {
            _fieldHandles.push_back(handle);
        }
    }
}

void
QueryCompletenessExecutor::execute(search::fef::MatchData &match)
{
    uint32_t hit = 0, miss = 0;
    for (std::vector<search::fef::TermFieldHandle>::iterator it = _fieldHandles.begin();
         it != _fieldHandles.end(); ++it)
    {
        search::fef::TermFieldMatchData &tfmd = *match.resolveTermField(*it);
        if (tfmd.getDocId() == match.getDocId()) {
            search::fef::FieldPositionsIterator field = tfmd.getIterator();
            while (field.valid() && field.getPosition() < _config.fieldBegin) {
                field.next();
            }
            if (field.valid() && field.getPosition() < _config.fieldEnd) {
                ++hit;
            } else {
                ++miss;
            }
        } else {
            ++miss;
        }
    }
    outputs().set_number(0, hit);
    outputs().set_number(1, miss);
}

QueryCompletenessBlueprint::QueryCompletenessBlueprint() :
    search::fef::Blueprint("queryCompleteness"),
    _config()
{
    // empty
}

void
QueryCompletenessBlueprint::visitDumpFeatures(const search::fef::IIndexEnvironment &,
                                              search::fef::IDumpFeatureVisitor &) const
{
    // empty
}

bool
QueryCompletenessBlueprint::setup(const search::fef::IIndexEnvironment &env,
                                  const search::fef::ParameterList &params)
{
    _config.fieldId = params[0].asField()->id();
    if (params.size() > 1) {
        _config.fieldBegin = params[1].asInteger();
        if (params.size() == 3) {
            _config.fieldEnd = params[2].asInteger();
        }
        if (_config.fieldBegin >= _config.fieldEnd) {
            LOG(error, "Can not calculate query completeness for field '%s' because range is malformed (from %d to %d).",
                params[0].getValue().c_str(), _config.fieldBegin, _config.fieldEnd);
            return false;
        }
    }
    describeOutput("hit",  "The number of query terms matched in field.");
    describeOutput("miss", "The number of query terms not matched in field.");
    env.hintFieldAccess(_config.fieldId);
    return true;
}

search::fef::Blueprint::UP
QueryCompletenessBlueprint::createInstance() const
{
    return search::fef::Blueprint::UP(new QueryCompletenessBlueprint());
}

search::fef::FeatureExecutor &
QueryCompletenessBlueprint::createExecutor(const search::fef::IQueryEnvironment &env, vespalib::Stash &stash) const
{
    return stash.create<QueryCompletenessExecutor>(env, _config);
}

}}
