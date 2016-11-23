// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/storage/distributor/managed_bucket_space_component.h>

namespace storage {
namespace distributor {

ManagedBucketSpaceComponent::ManagedBucketSpaceComponent(
        DistributorInterface& distributor,
        ManagedBucketSpace& bucketSpace,
        DistributorComponentRegister& compReg,
        const std::string& name)
    : DistributorComponent(distributor, compReg, name),
      _bucketSpace(bucketSpace)
{
}

} // distributor
} // storage
