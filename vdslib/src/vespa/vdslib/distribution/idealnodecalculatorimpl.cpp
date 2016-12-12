// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "idealnodecalculatorimpl.h"
#include <vespa/vespalib/util/exceptions.h>

namespace storage {
namespace lib {

IdealNodeCalculatorImpl::IdealNodeCalculatorImpl()
    : _distribution(0),
      _clusterState(0)
{
    initUpStateMapping();
}

IdealNodeCalculatorImpl::~IdealNodeCalculatorImpl() { }

void
IdealNodeCalculatorImpl::setDistribution(const Distribution& d) {
    _distribution = &d;
}
void
IdealNodeCalculatorImpl::setClusterState(const ClusterState& cs) {
    _clusterState = &cs;
}

IdealNodeList
IdealNodeCalculatorImpl::getIdealNodes(const NodeType& nodeType,
                                       const document::BucketId& bucket,
                                       UpStates upStates) const
{
    assert(_clusterState != 0);
    assert(_distribution != 0);
    std::vector<uint16_t> nodes;
    _distribution->getIdealNodes(nodeType, *_clusterState, bucket, nodes, _upStates[upStates]);
    IdealNodeList list;
    for (uint32_t i=0; i<nodes.size(); ++i) {
        list.push_back(Node(nodeType, nodes[i]));
    }
    return list;
}

void
IdealNodeCalculatorImpl::initUpStateMapping() {
    _upStates.clear();
    _upStates.resize(UP_STATE_COUNT);
    _upStates[UpInit] = "ui";
    _upStates[UpInitMaintenance] = "uim";
    for (uint32_t i=0; i<_upStates.size(); ++i) {
        if (_upStates[i] == 0) throw vespalib::IllegalStateException(
                "Failed to initialize up state. Code likely not updated "
                "after another upstate was added.", VESPA_STRLOC);
    }
}

} // lib
} // storage
