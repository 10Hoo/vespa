// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "clusterinformation.h"
#include <vespa/vdslib/distribution/distribution.h>
#include <vespa/vdslib/state/clusterstate.h>

namespace storage::distributor {

bool
ClusterInformation::nodeInSameGroupAsSelf(uint16_t otherNode) const
{
    return (getDistribution().getNodeGraph().getGroupForNode(otherNode)
            == getDistribution().getNodeGraph().getGroupForNode(getDistributorIndex()));
}

vespalib::string
ClusterInformation::getDistributionHash() const
{
    return getDistribution().getNodeGraph().getDistributionConfigHash();
}

std::vector<uint16_t>
ClusterInformation::getIdealStorageNodesForState(
        const lib::ClusterState& clusterState,
        const document::BucketId& bucketId) const
{
    return getDistribution().getIdealStorageNodes(
            clusterState,
            bucketId,
            getStorageUpStates());
}

uint16_t
ClusterInformation::getStorageNodeCount() const
{
    return getClusterState().getNodeCount(lib::NodeType::STORAGE);
}

}
