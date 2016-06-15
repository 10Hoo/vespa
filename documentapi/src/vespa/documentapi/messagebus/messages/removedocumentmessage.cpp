// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/documentapi/messagebus/documentprotocol.h>
#include <vespa/documentapi/messagebus/messages/removedocumentmessage.h>
#include <vespa/documentapi/messagebus/messages/removedocumentreply.h>

namespace documentapi {

RemoveDocumentMessage::RemoveDocumentMessage() :
    TestAndSetMessage(),
    _documentId()
{
    // empty
}

RemoveDocumentMessage::RemoveDocumentMessage(const document::DocumentId& documentId) :
    TestAndSetMessage(),
    _documentId(documentId)
{
    // empty
}

DocumentReply::UP
RemoveDocumentMessage::doCreateReply() const
{
    return DocumentReply::UP(new RemoveDocumentReply());
}

bool
RemoveDocumentMessage::hasSequenceId() const
{
    return true;
}

uint64_t
RemoveDocumentMessage::getSequenceId() const
{
    return *reinterpret_cast<const uint64_t*>(_documentId.getGlobalId().get());
}

uint32_t
RemoveDocumentMessage::getType() const
{
    return DocumentProtocol::MESSAGE_REMOVEDOCUMENT;
}

const document::DocumentId&
RemoveDocumentMessage::getDocumentId() const
{
    return _documentId;
}

void
RemoveDocumentMessage::setDocumentId(const document::DocumentId& documentId)
{
    _documentId = documentId;
}

}
