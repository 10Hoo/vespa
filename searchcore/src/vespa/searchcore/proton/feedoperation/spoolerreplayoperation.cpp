// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".proton.feedoperation.spoolerreplayoperation");

#include "spoolerreplayoperation.h"
#include <vespa/vespalib/util/stringfmt.h>

using vespalib::make_string;

namespace proton {


SpoolerReplayOperation::SpoolerReplayOperation(Type type)
    : FeedOperation(type),
    _spoolerSerialNum()
{
}


SpoolerReplayOperation::SpoolerReplayOperation(Type type,
                                               SerialNum serialNum,
                                               SerialNum spoolerSerialNum)
    : FeedOperation(type),
    _spoolerSerialNum(spoolerSerialNum)
{
    setSerialNum(serialNum);
}


void
SpoolerReplayOperation::serialize(vespalib::nbostream &os) const
{
    LOG(debug, "serialize(): %s", toString().c_str());
    os << _spoolerSerialNum;
}


void
SpoolerReplayOperation::deserialize(vespalib::nbostream &is)
{
    is >> _spoolerSerialNum;
}

vespalib::string SpoolerReplayOperation::toString() const {
    return make_string(
            "SpoolerReplay%s(spoolerSerialNum=%" PRIu64
            ", serialNum=%" PRIu64 ")",
            getType() == SPOOLER_REPLAY_START ? "Start" : "Complete",
            _spoolerSerialNum, getSerialNum());
}


SpoolerReplayStartOperation::SpoolerReplayStartOperation()
    : SpoolerReplayOperation(FeedOperation::SPOOLER_REPLAY_START)
{
}


SpoolerReplayStartOperation::SpoolerReplayStartOperation(SerialNum serialNum,
                                                         SerialNum spoolerSerialNum)
    : SpoolerReplayOperation(FeedOperation::SPOOLER_REPLAY_START,
                             serialNum,
                             spoolerSerialNum)
{
}


SpoolerReplayCompleteOperation::SpoolerReplayCompleteOperation()
    : SpoolerReplayOperation(FeedOperation::SPOOLER_REPLAY_COMPLETE)
{
}


SpoolerReplayCompleteOperation::SpoolerReplayCompleteOperation(SerialNum serialNum,
                                                               SerialNum spoolerSerialNum)
    : SpoolerReplayOperation(FeedOperation::SPOOLER_REPLAY_COMPLETE,
                             serialNum,
                             spoolerSerialNum)
{
}



} // namespace proton
