// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "configvalidator.h"

namespace proton {

/**
 * Class used to validate new schema before starting using it.
 **/
struct SchemaConfigValidator
{
    static ConfigValidator::Result
    validate(const search::index::Schema &newSchema,
             const search::index::Schema &oldSchema,
             const search::index::Schema &oldHistory);
};

} // namespace proton

