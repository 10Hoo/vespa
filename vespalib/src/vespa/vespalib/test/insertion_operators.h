// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <ostream>
#include <set>
#include <vector>

namespace std {

template <typename T>
std::ostream &
operator<<(std::ostream &os, const std::set<T> &set)
{
    os << "{";
    bool first = true;
    for (const auto &entry : set) {
        if (!first) {
            os << ",";
        }
        os << entry;
        first = false;
    }
    os << "}";
    return os;
}

template <typename T>
std::ostream &
operator<<(std::ostream &os, const std::vector<T> &set)
{
    os << "[";
    bool first = true;
    for (const auto &entry : set) {
        if (!first) {
            os << ",";
        }
        os << entry;
        first = false;
    }
    os << "]";
    return os;
}

} // namespace std

