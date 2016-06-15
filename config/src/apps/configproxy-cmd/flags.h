// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/vespalib/stllike/string.h>
#include <vector>

struct Flags {
    vespalib::string method;
    std::vector<vespalib::string> args;
    vespalib::string hostname;
    int portnumber;
    Flags() : method("cache"), args(), hostname("localhost"), portnumber(19090) {}
};

