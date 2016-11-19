// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/storage/distributor/latency_statistics_provider.h>
#include <ostream>

namespace storage {
namespace distributor {

std::ostream&
operator<<(std::ostream& os, const OperationStats& op)
{
    os << "OperationStats("
       << "totalLatency=" << op.totalLatency.count()
       << "ms, numRequests=" << op.numRequests
       << ')';
    return os;
}

std::ostream&
operator<<(std::ostream& os, const NodeStats& stats)
{
    os << "NodeStats(puts=" << stats.puts << ')';
    return os;
}

} // distributor
} // storage

