// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <limits>
#include <vespa/storageapi/messageapi/storagecommand.h>
#include <vespa/vespalib/util/exceptions.h>

namespace storage {
namespace api {

StorageCommand::StorageCommand(const StorageCommand& other)
    : StorageMessage(other, generateMsgId()),
      _timeout(other._timeout),
      _sourceIndex(other._sourceIndex)
{
    setTrace(other.getTrace());
}

StorageCommand::StorageCommand(const MessageType& type, Priority p)
    : StorageMessage(type, generateMsgId()),
        // Default timeout is unlimited. Set from mbus message. Some internal
        // use want unlimited timeout, (such as readbucketinfo, repair bucket
        // etc)
      _timeout(std::numeric_limits<uint32_t>().max()),
      _sourceIndex(0xFFFF)
{
    setPriority(p);
}

void
StorageCommand::print(std::ostream& out, bool verbose,
                      const std::string& indent) const
{
    (void) verbose; (void) indent;
    out << "StorageCommand(" << _type.getName();
    if (_priority != NORMAL) out << ", priority = " << static_cast<int>(_priority);
    if (_sourceIndex != 0xFFFF) out << ", source = " << _sourceIndex;
    out << ", timeout = " << _timeout << " ms";
    out << ")";
}

StorageCommand::UP
StorageCommand::createCopyToForward(const document::BucketId&, uint64_t) const
{
    throw vespalib::IllegalStateException(
            "Command " + _type.getName() + " does not support forwarding.",
            VESPA_STRLOC);
}

} // api
} // storage
