// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "address_space_usage_stats.h"

namespace proton {

AddressSpaceUsageStats::AddressSpaceUsageStats(const search::AddressSpace &
                                               usage)
    : _usage(usage),
      _attributeName(),
      _subDbName()
{
}

void
AddressSpaceUsageStats::merge(const search::AddressSpace &usage,
                              const vespalib::string &attributeName,
                              const vespalib::string &subDbName)
{
    if (attributeName.empty() || usage.usage() > _usage.usage()) {
        _usage = usage;
        _attributeName = attributeName;
        _subDbName = subDbName;
    }
}


} // namespace proton
