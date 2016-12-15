// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "hash_set.hpp"

namespace vespalib {
}

VESPALIB_HASH_SET_INSTANTIATE(int32_t);
VESPALIB_HASH_SET_INSTANTIATE(uint32_t);
VESPALIB_HASH_SET_INSTANTIATE(uint64_t);
VESPALIB_HASH_SET_INSTANTIATE(double);
VESPALIB_HASH_SET_INSTANTIATE(vespalib::string);
VESPALIB_HASH_SET_INSTANTIATE(std::string);
VESPALIB_HASH_SET_INSTANTIATE(const void *);