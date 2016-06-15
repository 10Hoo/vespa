// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/storage/common/messagesender.h>
#include <vespa/storageapi/messageapi/storagemessage.h>
#include <vespa/storageapi/messageapi/storagereply.h>
#include <vespa/storageapi/messageapi/storagecommand.h>

namespace storage {

void
MessageSender::send(const std::shared_ptr<api::StorageMessage>& msg)
{
    if (msg->getType().isReply()) {
        sendReply(std::static_pointer_cast<api::StorageReply>(msg));
    } else {
        sendCommand(std::static_pointer_cast<api::StorageCommand>(msg));
    }
}

}
