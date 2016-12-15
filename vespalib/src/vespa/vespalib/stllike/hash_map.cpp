// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "hash_map.hpp"

namespace vespalib {
}

VESPALIB_HASH_MAP_INSTANTIATE(vespalib::string, vespalib::string);
VESPALIB_HASH_MAP_INSTANTIATE(vespalib::string, int32_t);
VESPALIB_HASH_MAP_INSTANTIATE(vespalib::string, uint32_t);
VESPALIB_HASH_MAP_INSTANTIATE(vespalib::string, uint64_t);
VESPALIB_HASH_MAP_INSTANTIATE(vespalib::string, double);
VESPALIB_HASH_MAP_INSTANTIATE(int64_t, int32_t);
VESPALIB_HASH_MAP_INSTANTIATE(uint32_t, int32_t);
VESPALIB_HASH_MAP_INSTANTIATE(uint32_t, uint32_t);
VESPALIB_HASH_MAP_INSTANTIATE(uint64_t, uint32_t);
VESPALIB_HASH_MAP_INSTANTIATE(double, uint32_t);
