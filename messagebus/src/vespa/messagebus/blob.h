// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/vespalib/util/alloc.h>

namespace mbus {

/**
 * This class encapsulates a blob that is owned by the object. Objects
 * of this class have destructive copy. Use Blob objects when you want
 * to transfer the ownership of a blob, like when it is returned by a
 * method.
 **/
class Blob
{
public:
    /**
     * Create a blob that will contain uninitialized memory with the
     * given size.
     *
     * @param s size of the data to be created
     **/
    Blob(uint32_t s) :
        _payload(s),
        _sz(s)
    { }
    Blob(Blob && rhs) :
        _payload(std::move(rhs._payload)),
        _sz(rhs._sz)
    {
        rhs._sz = 0;
    }
    Blob & operator = (Blob && rhs) {
        swap(rhs);
        return *this;
    }

    void swap(Blob & rhs) {
        _payload.swap(rhs._payload);
        std::swap(_sz, rhs._sz);
    }

    /**
     * Obtain the data owned by this Blob
     *
     * @return data
     **/
    char *data() { return static_cast<char *>(_payload.get()); }

    /**
     * Obtain the data owned by this Blob
     *
     * @return data
     **/
    const char *data() const { return static_cast<const char *>(_payload.get()); }

    vespalib::DefaultAlloc & payload() { return _payload; }
    const vespalib::DefaultAlloc & payload() const { return _payload; }
    size_t size() const { return _sz; }
private:
    vespalib::DefaultAlloc _payload;
    size_t _sz;
};

} // namespace mbus

