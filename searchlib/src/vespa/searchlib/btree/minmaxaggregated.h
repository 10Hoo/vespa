// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <limits>
#include <stdint.h>

namespace search {
namespace btree {

class MinMaxAggregated
{
    int32_t _min;
    int32_t _max;
    
public:
    MinMaxAggregated(void)
        : _min(std::numeric_limits<int32_t>::max()),
          _max(std::numeric_limits<int32_t>::min())
    { }

    MinMaxAggregated(int32_t min, int32_t max)
        : _min(min),
          _max(max)
    { }

    int32_t getMin(void) const { return _min; }
    int32_t getMax(void) const { return _max; }

    void
    add(int32_t val)
    {
        if (_min > val)
            _min = val;
        if (_max < val)
            _max = val;
    }

    void
    add(const MinMaxAggregated &ca)
    {
        if (_min > ca._min)
            _min = ca._min;
        if (_max < ca._max)
            _max = ca._max;
    }

    void
    add(const MinMaxAggregated &oldca,
        const MinMaxAggregated &ca)
    {
        (void) oldca;
        add(ca);
    }

    /* Returns true if recalculation is needed */
    bool
    remove(int32_t val)
    {
        return (_min == val || _max == val);
    }

    /* Returns true if recalculation is needed */
    bool
    remove(const MinMaxAggregated &oldca,
           const MinMaxAggregated &ca)
    {
        return (_min == oldca._min && _min != ca._min) ||
            (_max == oldca._max && _max != ca._max);
    }

    /* Returns true if recalculation is needed */
    bool
    update(int32_t oldVal, int32_t val)
    {
        if ((_min == oldVal && _min < val) ||
            (_max == oldVal && _max > val)) {
            return true;
        }
        add(val);
        return false;
    }

    /* Returns true if recalculation is needed */
    bool
    update(const MinMaxAggregated &oldca,
           const MinMaxAggregated &ca)
    {
        if ((_min == oldca._min && _min < ca._min) ||
            (_max == oldca._max && _max > ca._max)) {
            return true;
        }
        add(ca);
        return false;
    }
};


} // namespace search::btree
} // namespace search

