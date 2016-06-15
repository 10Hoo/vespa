// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".document.serializable-array");

#include <vespa/document/fieldvalue/serializablearray.h>

#include <vespa/document/util/bytebuffer.h>
#include <vespa/document/util/compressor.h>
#include <vespa/vespalib/util/stringfmt.h>
#include <algorithm>
#include <string>
#include <vector>

using std::vector;

namespace document {

SerializableArray::Statistics SerializableArray::_stats;

SerializableArray::SerializableArray()
    : _serializedCompression(CompressionConfig::NONE),
      _uncompressedLength(0)
{
}

SerializableArray::SerializableArray(const SerializableArray& other)
    : Cloneable(),
      _entries(other._entries),
      _owned(),
      _uncompSerData(other._uncompSerData.get() ? new ByteBuffer(*other._uncompSerData) : NULL),
      _compSerData(other._compSerData.get() ? new ByteBuffer(*other._compSerData) : NULL),
      _serializedCompression(other._serializedCompression),
      _uncompressedLength(other._uncompressedLength)
{
    for (size_t i(0); i < _entries.size(); i++) {
        Entry & e(_entries[i]);
        if (e.hasBuffer()) {
            // Pointing to a buffer in the _owned structure.
            ByteBuffer::UP buf(ByteBuffer::copyBuffer(e.getBuffer(_uncompSerData.get()), e.size()));
            e.setBuffer(buf->getBuffer());
            _owned[e.id()] = std::move(buf);
        } else {
            // If not it is relative to the buffer _uncompSerData, and hence it is valid as is.
        }
    }
    if (_uncompSerData.get()) {
        LOG_ASSERT(_uncompressedLength == _uncompSerData->getRemaining());
    }
}

void
SerializableArray::swap(SerializableArray& other)
{
    _entries.swap(other._entries);
    _owned.swap(other._owned);
    std::swap(_uncompSerData, other._uncompSerData);
    std::swap(_compSerData, other._compSerData);
    std::swap(_serializedCompression, other._serializedCompression);
    std::swap(_uncompressedLength, other._uncompressedLength);
}

void SerializableArray::clear()
{
    _entries.clear();
    _uncompSerData.reset();
    _compSerData.reset();
    _serializedCompression = CompressionConfig::NONE;
    _uncompressedLength = 0;
}

SerializableArray::~SerializableArray()
{
}

void
SerializableArray::invalidate()
{
    _compSerData.reset();
}

void
SerializableArray::set(int id, ByteBuffer::UP buffer)
{
    maybeDecompress();
    Entry e(id, buffer->getRemaining(), buffer->getBuffer());
    _owned[id] = std::move(buffer);
    EntryMap::iterator it = find(id);
    if (it == _entries.end()) {
        _entries.push_back(e);
    } else {
        *it = e;
    }
    invalidate();
}

void SerializableArray::set(int id, const char* value, int len)
{
    set(id, std::unique_ptr<ByteBuffer>(ByteBuffer::copyBuffer(value,len)));
}

SerializableArray::EntryMap::const_iterator
SerializableArray::find(int id) const
{
    return std::find_if(_entries.begin(), _entries.end(), [id](const auto& e){ return e.id() == id; });
}

SerializableArray::EntryMap::iterator
SerializableArray::find(int id)
{
    return std::find_if(_entries.begin(), _entries.end(), [id](const auto& e){ return e.id() == id; });
}

bool
SerializableArray::has(int id) const
{
    return (find(id) != _entries.end());
}

vespalib::ConstBufferRef
SerializableArray::get(int id) const
{
    vespalib::ConstBufferRef buf;
    if ( !maybeDecompressAndCatch() ) {
        EntryMap::const_iterator found = find(id);

        if (found != _entries.end()) {
            const Entry& entry = *found;
            buf = vespalib::ConstBufferRef(entry.getBuffer(_uncompSerData.get()), entry.size());
        }
    } else {
        // should we clear all or what?
    }

    return buf;
}

bool
SerializableArray::deCompressAndCatch() const
{
    try {
        const_cast<SerializableArray *>(this)->deCompress();
        return false;
    } catch (const std::exception & e) {
        LOG(warning, "Deserializing compressed content failed: %s", e.what());
        return true;
    }
}

void
SerializableArray::clear(int id)
{
    maybeDecompress();
    EntryMap::iterator it = find(id);
    if (it != _entries.end()) {
        _entries.erase(it);
        _owned.erase(id);
        invalidate();
    }
}

void
SerializableArray::deCompress() // throw (DeserializeException)
{
    // will only do this once

    LOG_ASSERT(_compSerData);
    LOG_ASSERT(!_uncompSerData);

    if (_serializedCompression == CompressionConfig::NONE ||
        _serializedCompression == CompressionConfig::UNCOMPRESSABLE)
    {
        _uncompSerData = std::move(_compSerData);
        LOG_ASSERT(_uncompressedLength == _uncompSerData->getRemaining());
    } else {
        ByteBuffer::UP newSerialization(new ByteBuffer(_uncompressedLength));
        vespalib::DataBuffer unCompressed(newSerialization->getBuffer(), newSerialization->getLength());
        unCompressed.clear();
        try {
            decompress(_serializedCompression,
                       _uncompressedLength,
                       vespalib::ConstBufferRef(_compSerData->getBufferAtPos(), _compSerData->getRemaining()),
                       unCompressed,
                       false);
        } catch (const std::runtime_error & e) {
            throw DeserializeException(
                vespalib::make_string( "Document was compressed with code unknown code %d", _serializedCompression),
                VESPA_STRLOC);
        }

        if (unCompressed.getDataLen() != (size_t)_uncompressedLength) {
            throw DeserializeException(
                    vespalib::make_string(
                            "Did not decompress to the expected length: had %" PRIu64 ", wanted %" PRId32 ", got %" PRIu64,
                            _compSerData->getRemaining(), _uncompressedLength, unCompressed.getDataLen()),
                    VESPA_STRLOC);
        }
        assert(newSerialization->getBuffer() == unCompressed.getData());
        newSerialization->setLimit(_uncompressedLength);
        _uncompSerData = std::move(newSerialization);
        LOG_ASSERT(_uncompressedLength == _uncompSerData->getRemaining());
    }
}

void SerializableArray::assign(EntryMap & entries,
                               ByteBuffer::UP buffer,
                               CompressionConfig::Type comp_type,
                               uint32_t uncompressed_length)
{
    _serializedCompression = comp_type;

    _entries.clear();
    _entries.swap(entries);
    if (CompressionConfig::isCompressed(_serializedCompression)) {
        _compSerData.reset(buffer.release());
        _uncompressedLength = uncompressed_length;
    } else {
        _uncompressedLength = buffer->getRemaining();
        _uncompSerData.reset(buffer.release());
    }
}

} // document
