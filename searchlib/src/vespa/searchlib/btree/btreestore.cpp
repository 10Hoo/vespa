// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "btreestore.h"
#include "datastore.h"
#include "btreenode.h"
#include "btreerootbase.h"
#include "btreeroot.h"
#include "btreenodeallocator.h"
#include "btreeiterator.hpp"
#include "btreestore.hpp"

namespace search
{

namespace btree
{

template class BTreeStore<uint32_t, uint32_t,
                          NoAggregated,
                          std::less<uint32_t>,
                          BTreeDefaultTraits>;

template class BTreeStore<uint32_t, BTreeNoLeafData,
                          NoAggregated,
                          std::less<uint32_t>,
                          BTreeDefaultTraits>;

template class BTreeStore<uint32_t, int32_t,
                          MinMaxAggregated,
                          std::less<uint32_t>,
                          BTreeDefaultTraits,
                          MinMaxAggrCalc>;

} // namespace btree

} // namespace search
