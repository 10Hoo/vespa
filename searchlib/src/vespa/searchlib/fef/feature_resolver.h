// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "number_or_object.h"
#include <vespa/vespalib/stllike/string.h>
#include <vector>

namespace search {
namespace fef {

class RankProgram;

/**
 * A FeatureResolver knowns the name and memory location of values
 * calculated by a RankProgram. Note that objects of this class will
 * reference data owned by the RankProgram used to create it.
 **/
class FeatureResolver
{
private:
    friend class RankProgram; // inner class with decoupled compilation
    std::vector<vespalib::string> _names;
    std::vector<const NumberOrObject *> _features;
    std::vector<bool> _is_object;
public:
    FeatureResolver(size_t size_hint) : _names(), _features(), _is_object() {
        _names.reserve(size_hint);
        _features.reserve(size_hint);
        _is_object.reserve(size_hint);
    }
    void add(const vespalib::string &name, const NumberOrObject *feature, bool is_object) {
        _names.push_back(name);
        _features.push_back(feature);
        _is_object.push_back(is_object);
    }
    size_t num_features() const { return _names.size(); }
    const vespalib::string &name_of(size_t i) const { return _names[i]; }
    bool is_object(size_t i) const { return _is_object[i]; }
    const feature_t *resolve_number(size_t i) const { return &(_features[i]->as_number); }
    const vespalib::eval::Value::CREF *resolve_object(size_t i) const { return &(_features[i]->as_object); }
    const NumberOrObject *resolve_raw(size_t i) const { return _features[i]; }
};

} // namespace fef
} // namespace search
