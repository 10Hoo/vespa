// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchlib/common/feature.h>
#include <vespa/vespalib/eval/value.h>

namespace search {
namespace fef {

/**
 * Storage cell for values passed between feature executors in the
 * ranking framework. The union either contains a double value
 * directly (number) or a reference to a polymorphic value stored
 * elsewhere (object).
 **/
union NumberOrObject {
    feature_t                   as_number;
    vespalib::eval::Value::CREF as_object;
    NumberOrObject() { memset(this, 0, sizeof(NumberOrObject)); }
    ~NumberOrObject() {}
};

} // namespace fef
} // namespace search
