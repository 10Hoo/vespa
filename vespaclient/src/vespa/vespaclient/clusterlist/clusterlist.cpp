// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/vespaclient/clusterlist/clusterlist.h>
#include <vespa/config/config.h>
#include <sstream>

using namespace vespaclient;

VESPA_IMPLEMENT_EXCEPTION(VCClusterNotFoundException, vespalib::IllegalArgumentException);

ClusterList::ClusterList()
{
  configure(*config::ConfigGetter<cloud::config::ClusterListConfig>::getConfig("client"));
}

void
ClusterList::configure(const cloud::config::ClusterListConfig& config)
{
    _contentClusters.clear();
    for (uint32_t i = 0; i < config.storage.size(); i++) {
        _contentClusters.push_back(
                Cluster(config.storage[i].name, config.storage[i].configid));
    }
}

std::string
ClusterList::getContentClusterList() const
{
    std::ostringstream ost;

    for (uint32_t j = 0; j < _contentClusters.size(); j++) {
        if (j != 0) {
            ost << ",";
        }
        ost << _contentClusters[j].getName();
    }

    return ost.str();
}

const ClusterList::Cluster&
ClusterList::verifyContentCluster(const std::string& cluster) const
{
    if (cluster.length()) {
        for (uint32_t j = 0; j < _contentClusters.size(); j++) {
            if (_contentClusters[j].getName() == cluster) {
                return _contentClusters[j];
            }
        }

        std::ostringstream ost;
        ost << "Cluster " << cluster
            << " has not been configured in the vespa cluster. Legal clusters are ["
            << getContentClusterList() << "]";
        throw ClusterNotFoundException(ost.str());
    } else if (_contentClusters.size() == 1) {
        return _contentClusters[0];
    } else {
        std::ostringstream ost;
        ost << "No content cluster specified. Legal clusters are [" << getContentClusterList() << "]";
        throw ClusterNotFoundException(ost.str());
    }
}

