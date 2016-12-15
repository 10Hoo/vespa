// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/persistence/spi/context.h>

namespace storage {
namespace spi {

Context::Context(const LoadType& loadType, Priority pri, int maxTraceLevel)
    : _loadType(&loadType),
      _priority(pri),
      _trace(maxTraceLevel),
      _readConsistency(ReadConsistency::STRONG)
{ }

Context::~Context() { }

} // spi
} // storage
