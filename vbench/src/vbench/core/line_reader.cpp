// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "line_reader.h"

namespace vbench {

namespace {
void stripCR(string &dst) {
    if (!dst.empty() && dst[dst.size() - 1] == '\r') {
        dst.resize(dst.size() - 1);
    }
}
} // namespace vbench::<unnamed>

LineReader::LineReader(Input &input, size_t chunkSize)
    : _input(input, chunkSize)
{
}

bool
LineReader::readLine(string &dst)
{
    dst.clear();
    for (int c = _input.get(); c >= 0; c = _input.get()) {
        if (c != '\n') {
            dst.push_back(c);
        } else {
            stripCR(dst);
            return true;
        }
    }
    return !dst.empty();
}

} // namespace vbench
