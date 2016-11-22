// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/storage/common/distributorcomponent.h>

namespace storage {

DistributorComponent::DistributorComponent(DistributorComponentRegister& compReg,
                                           vespalib::stringref name)
    : StorageComponent(compReg, name),
      _bucketDatabase(0), _timeCalculator(0),
      _totalConfig(*this)
{
    compReg.registerDistributorComponent(*this);
}

DistributorComponent::~DistributorComponent() { }

} // storage

