// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "allocator.h"
#include "bufferstate.h"

namespace search {
namespace datastore {

template <typename EntryT, typename RefT>
Allocator<EntryT, RefT>::Allocator(DataStoreBase &store, uint32_t typeId)
    : _store(store),
      _typeId(typeId)
{
}

template <typename EntryT, typename RefT>
template <typename ... Args>
typename Allocator<EntryT, RefT>::HandleType
Allocator<EntryT, RefT>::alloc(Args && ... args)
{
    _store.ensureBufferCapacity(_typeId, 1);
    uint32_t activeBufferId = _store.getActiveBufferId(_typeId);
    BufferState &state = _store.getBufferState(activeBufferId);
    assert(state.isActive());
    size_t oldBufferSize = state.size();
    EntryT *entry = _store.getBufferEntry<EntryT>(activeBufferId, oldBufferSize);
    new (static_cast<void *>(entry)) EntryT(std::forward<Args>(args)...);
    state.pushed_back(1);
    return HandleType(RefT(oldBufferSize, activeBufferId), entry);
}

template <typename EntryT, typename RefT>
typename Allocator<EntryT, RefT>::HandleType
Allocator<EntryT, RefT>::allocArray(ConstArrayRef array)
{
    _store.ensureBufferCapacity(_typeId, array.size());
    uint32_t activeBufferId = _store.getActiveBufferId(_typeId);
    BufferState &state = _store.getBufferState(activeBufferId);
    assert(state.isActive());
    assert(state.getClusterSize() == array.size());
    size_t oldBufferSize = state.size();
    EntryT *buf = _store.template getBufferEntry<EntryT>(activeBufferId, oldBufferSize);
    for (size_t i = 0; i < array.size(); ++i) {
        new (static_cast<void *>(buf + i)) EntryT(array[i]);
    }
    state.pushed_back(array.size());
    assert((oldBufferSize % array.size()) == 0);
    return HandleType(RefT((oldBufferSize / array.size()), activeBufferId), buf);
}

template <typename EntryT, typename RefT>
typename Allocator<EntryT, RefT>::HandleType
Allocator<EntryT, RefT>::allocArray(size_t size)
{
    _store.ensureBufferCapacity(_typeId, size);
    uint32_t activeBufferId = _store.getActiveBufferId(_typeId);
    BufferState &state = _store.getBufferState(activeBufferId);
    assert(state.isActive());
    assert(state.getClusterSize() == size);
    size_t oldBufferSize = state.size();
    EntryT *buf = _store.template getBufferEntry<EntryT>(activeBufferId, oldBufferSize);
    for (size_t i = 0; i < size; ++i) {
        new (static_cast<void *>(buf + i)) EntryT();
    }
    state.pushed_back(size);
    assert((oldBufferSize % size) == 0);
    return HandleType(RefT((oldBufferSize / size), activeBufferId), buf);
}

}
}
