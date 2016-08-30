// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "constant_value_repo.h"
#include "error_constant_value.h"

using vespalib::eval::ConstantValue;

namespace proton {
namespace matching {

ConstantValueRepo::ConstantValueRepo(const ConstantValueFactory &factory)
    : _factory(factory),
      _constants()
{
}

void
ConstantValueRepo::reconfigure(const RankingConstants &constants)
{
    _constants = constants;
}

ConstantValue::UP
ConstantValueRepo::getConstant(const vespalib::string &name) const
{
    const RankingConstants::Constant *constant = _constants.getConstant(name);
    if (constant != nullptr) {
        return _factory.create(constant->filePath, constant->type);
    }
    return std::make_unique<ErrorConstantValue>();
}

}
}
