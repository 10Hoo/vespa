// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/log/log.h>
LOG_SETUP(".predicate_interval_store");
#include <vespa/fastos/fastos.h>

#include "predicate_interval_store.h"

#include "predicate_index.h"
#include <vespa/searchlib/btree/bufferstate.h>
#include <vespa/searchlib/btree/datastore.hpp>
#include <vespa/searchlib/btree/entryref.h>

using search::btree::BufferState;
using search::btree::EntryRef;
using std::vector;

namespace search {
namespace predicate {

template <typename T>
PredicateIntervalStore::Entry<T> PredicateIntervalStore::allocNewEntry(
        uint32_t type_id, uint32_t size) {
    _store.ensureBufferCapacity(type_id, size);
    uint32_t active_buffer_id = _store.getActiveBufferId(type_id);
    btree::BufferState &state = _store.getBufferState(active_buffer_id);
    assert(state._state == btree::BufferState::ACTIVE);
    size_t old_size = state.size();
    T *buf = _store.getBufferEntry<T>(active_buffer_id, old_size);
    state.pushed_back(size);
    return {RefType(old_size, active_buffer_id), buf};
}

PredicateIntervalStore::PredicateIntervalStore()
    : _store(),
      _size1Type(1, 1024u, RefType::offsetSize()),
      _store_adapter(_store),
      _ref_cache(_store_adapter) {

    // This order determines type ids.
    _store.addType(&_size1Type);

    _store.initActiveBuffers();
}

PredicateIntervalStore::~PredicateIntervalStore() {
    _store.dropBuffers();
}

//
// NOTE: The allocated entries are arrays of type uint32_t, but the
// entries are used as arrays of either Interval or IntervalWithBounds
// objects (PODs). These objects are memcpy'ed into the uint32_t
// arrays, and in the get() function they are typecast back to the
// object expected by the caller. Which type an entry has cannot be
// inferred from the EntryRef, but must be known by the caller.
//
// This saves us from having separate buffers for Intervals and
// IntervalWithBounds objects, since the caller knows the correct type
// anyway.
//
template <typename IntervalT>
btree::EntryRef PredicateIntervalStore::insert(
        const vector<IntervalT> &intervals) {
    const uint32_t size = entrySize<IntervalT>() * intervals.size();
    if (size == 0) {
        return btree::EntryRef();
    }
    uint32_t *buffer;
    btree::EntryRef ref;
    if (size == 1 && intervals[0].interval <= RefCacheType::DATA_REF_MASK) {
        return btree::EntryRef(intervals[0].interval);
    }
    uint32_t cached_ref = _ref_cache.find(
            reinterpret_cast<const uint32_t *>(&intervals[0]), size);
    if (cached_ref) {
        return cached_ref;
    }

    if (size < RefCacheType::MAX_SIZE) {
        auto entry = allocNewEntry<uint32_t>(0, size);
        buffer = entry.buffer;
        ref = entry.ref.ref() | (size << RefCacheType::SIZE_SHIFT);
    } else {
        auto entry = allocNewEntry<uint32_t>(0, size + 1);
        buffer = entry.buffer;
        ref = entry.ref.ref() | RefCacheType::SIZE_MASK;
        *buffer++ = size;
    }
    memcpy(buffer, &intervals[0], size * sizeof(uint32_t));
    _ref_cache.insert(ref.ref());
    return ref;
}
// Explicit instantiation for relevant types.
template
EntryRef PredicateIntervalStore::insert(const vector<Interval> &);
template
EntryRef PredicateIntervalStore::insert(const vector<IntervalWithBounds> &);

void PredicateIntervalStore::remove(EntryRef ref) {
    if (ref.valid()) {
        uint32_t buffer_id = RefType(ref).bufferId();
        if (buffer_id == 0) {  // single interval optimization.
            return;
        }
        // Don't remove anything.

        // BufferState &state = _store.getBufferState(buffer_id);
        // uint32_t type_id = state.getTypeId();
        // uint32_t size = type_id <= MAX_ARRAY_SIZE ? type_id : 1;
        // _store.holdElem(ref, size);
    }
}

void PredicateIntervalStore::trimHoldLists(generation_t used_generation) {
    _store.trimHoldLists(used_generation);
}

void PredicateIntervalStore::transferHoldLists(generation_t generation) {
    _store.transferHoldLists(generation);
}

}  // namespace predicate
}  // namespace search
