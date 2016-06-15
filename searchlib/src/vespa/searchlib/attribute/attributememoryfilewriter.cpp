// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "attributememoryfilewriter.h"
#include "attributememoryfilebufferwriter.h"

namespace search
{

namespace
{

const uint32_t MIN_ALIGNMENT = 4096;

}

AttributeMemoryFileWriter::AttributeMemoryFileWriter()
    : IAttributeFileWriter(),
      _bufs()
{
}


AttributeMemoryFileWriter::~AttributeMemoryFileWriter()
{
}


AttributeMemoryFileWriter::Buffer
AttributeMemoryFileWriter::allocBuf(size_t size)
{
    return std::make_unique<BufferBuf>(size, MIN_ALIGNMENT);
}


void
AttributeMemoryFileWriter::writeBuf(Buffer buf)
{
    _bufs.emplace_back(std::move(buf));
}


void
AttributeMemoryFileWriter::writeTo(IAttributeFileWriter &writer)
{
    for (auto &buf : _bufs) {
        writer.writeBuf(std::move(buf));
    }
    _bufs.clear();
}


std::unique_ptr<BufferWriter>
AttributeMemoryFileWriter::allocBufferWriter()
{
    return std::make_unique<AttributeMemoryFileBufferWriter>(*this);
}


} // namespace search
