// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/storageapi/message/searchresult.h>

using vdslib::SearchResult;

namespace storage {
namespace api {

IMPLEMENT_COMMAND(SearchResultCommand, SearchResultReply)
IMPLEMENT_REPLY(SearchResultReply)

SearchResultCommand::SearchResultCommand()
    : StorageCommand(MessageType::SEARCHRESULT),
      SearchResult()
{
}

void
SearchResultCommand::print(std::ostream& out, bool verbose,
                           const std::string& indent) const
{
    out << "SearchResultCommand(" << getHitCount() << " hits)";
    if (verbose) {
        out << " : ";
        StorageCommand::print(out, verbose, indent);
    }
}

SearchResultReply::SearchResultReply(const SearchResultCommand& cmd)
    : StorageReply(cmd)
{
}

void
SearchResultReply::print(std::ostream& out, bool verbose,
                         const std::string& indent) const
{
    out << "SearchResultReply()";
    if (verbose) {
        out << " : ";
        StorageReply::print(out, verbose, indent);
    }
}

} // api
} // storage
