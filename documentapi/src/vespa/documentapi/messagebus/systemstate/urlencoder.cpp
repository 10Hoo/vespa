// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>

#include <vespa/vespalib/util/stringfmt.h>
#include <vespa/vespalib/stllike/asciistream.h>
#include "urlencoder.h"

LOG_SETUP(".urlencoder");

using namespace documentapi;

const string
URLEncoder::encode(const string &str)
{
    vespalib::asciistream out;
    for (size_t i = 0; i < str.size(); i++) {
        char c = str[i];
        if ((c >= 48 && c <= 57) ||  // The range '0'-'9'.
            (c >= 65 && c <= 90) ||  // The range 'A'-'Z'.
            (c >= 97 && c <= 122) || // The range 'a'-'z'.
            (c == '-' || c == '.' || c == '*' || c == '_')) {
            out << c;
        }
        else if (c == ' ') {
            out << '+';
        }
        else {
            out << "%" << vespalib::make_string("%02X", c & 0xff);
        }
    }
    return out.str();
}
