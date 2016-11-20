// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/storage/distributor/maintenance/node_maintenance_stats_tracker.h>
#include <ostream>

namespace storage {
namespace distributor {

const NodeMaintenanceStats NodeMaintenanceStatsTracker::_emptyStats;

std::ostream&
operator<<(std::ostream& os, const NodeMaintenanceStats& stats)
{
    os << "NodeStats("
       << "movingOut="   << stats.movingOut
       << ",syncing="    << stats.syncing
       << ",copyingIn="  << stats.copyingIn
       << ",copyingOut=" << stats.copyingOut
       << ")";
    return os;
}

} // distributor
} // storage

