// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/storage/distributor/operationowner.h>
#include <vespa/storage/distributor/operations/operation.h>
#include <vespa/log/log.h>
#include <vespa/storageapi/messageapi/storagemessage.h>
#include <vespa/storageapi/messageapi/storagecommand.h>
#include <vespa/storageapi/messageapi/storagereply.h>

LOG_SETUP(".operationowner");

namespace storage {

namespace distributor {

OperationOwner::~OperationOwner()
{
}

void
OperationOwner::Sender::sendCommand(const std::shared_ptr<api::StorageCommand> & msg)
{
    _owner.getSentMessageMap().insert(msg->getMsgId(), _cb);
    _sender.sendCommand(msg);
}

void
OperationOwner::Sender::sendReply(const std::shared_ptr<api::StorageReply> & msg)
{
    _sender.sendReply(msg);
};

bool
OperationOwner::handleReply(const std::shared_ptr<api::StorageReply>& reply)
{
    std::shared_ptr<Operation> cb = _sentMessageMap.pop(reply->getMsgId());

    if (cb.get() != 0) {
        Sender sender(*this, _sender, cb);
        cb->receive(sender, reply);
        return true;
    }

    return false;
}

bool
OperationOwner::start(const std::shared_ptr<Operation>& operation,
                      Priority priority)
{
    (void) priority;
    LOG(spam, "Starting operation %s", operation->toString().c_str());
    Sender sender(*this, _sender, operation);
    operation->start(sender, _clock.getTimeInMillis());
    return true;
}

std::string
OperationOwner::toString() const
{
    return _sentMessageMap.toString();
}

void
OperationOwner::onClose()
{
    while (true) {
        std::shared_ptr<Operation> cb = _sentMessageMap.pop();

        if (cb.get()) {
            Sender sender(*this, _sender, std::shared_ptr<Operation>());
            cb->onClose(sender);
        } else {
            break;
        }
    }
}

void
OperationOwner::erase(api::StorageMessage::Id msgId)
{
    _sentMessageMap.pop(msgId);
}


} // distributor

} // storage
