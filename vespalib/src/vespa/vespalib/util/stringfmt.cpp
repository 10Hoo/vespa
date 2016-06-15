// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "stringfmt.h"
#include "vstringfmt.h"

namespace vespalib {

//-----------------------------------------------------------------------------

vespalib::string make_vespa_string_va(const char *fmt, va_list ap)
{
    va_list ap2;
    vespalib::string ret;
    int size = -1;

    va_copy(ap2, ap);
    size = vsnprintf(ret.begin(), ret.capacity(), fmt, ap2);
    va_end(ap2);

    assert(size >= 0);
    if (ret.capacity() > static_cast<size_t>(size)) {
        // all OK
    } else {
        int newLen = size;
        ret.reserve(size+1);
        va_copy(ap2, ap);
        size = vsnprintf(ret.begin(), ret.capacity(), fmt, ap2);
        va_end(ap2);
        assert(newLen == size);
        (void)newLen;
    }
    ret.append_from_reserved(size);
    return ret;
}

/**
 * @brief construct string value printf style.
 *
 * You must \#include <vespa/vespalib/util/stringfmt.h>
 * to use this utility function.
 * @param fmt format string
 * @return formatted vespalib::string
 **/
vespalib::string make_vespa_string(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vespalib::string ret = make_vespa_string_va(fmt, ap);
    va_end(ap);
    return ret;
}

vespalib::string make_string(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vespalib::string ret = make_vespa_string_va(fmt, ap);
    va_end(ap);
    return ret;
}

vespalib::string make_string_va(const char *fmt, va_list ap)
{
    return make_vespa_string_va(fmt, ap);
}

} // namespace vespalib
