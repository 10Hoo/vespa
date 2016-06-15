// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "range.h"
#include <sstream>
#include <vespa/vespalib/stllike/asciistream.h>

namespace search {
namespace query {

Range::Range(int64_t f, int64_t t)
{
    vespalib::asciistream ost;
    ost << "[" << f << ";" << t << "]";
    _range = ost.str();
}

vespalib::asciistream &operator<<(vespalib::asciistream &out, const Range &range)
{
    return out << range.getRangeString();
}

}  // namespace query
}  // namespace search
