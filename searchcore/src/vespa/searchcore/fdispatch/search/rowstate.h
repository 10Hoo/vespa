// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vector>
#include <stdint.h>
#include <stdlib.h>
#include "poss_count.h"

namespace fdispatch {

/**
 * RowState keeps track of state per row or rather group.
 * Currently it just keeps the average searchtime as exponential decay.
 **/
class RowState {
public:
    RowState(double initialValue, uint64_t decayRate) :
        _decayRate(std::max(1ul, decayRate)),
        _avgSearchTime(initialValue),
        _sumActiveDocs(0),
        _numQueries(0)
    { }
    double getAverageSearchTime() const { return _avgSearchTime; }
    double getAverageSearchTimeInverse() const { return 1.0/_avgSearchTime; }
    void updateSearchTime(double searchTime);
    void setAverageSearchTime(double avgSearchTime) { _avgSearchTime = avgSearchTime; }
    uint64_t activeDocs() const { return _sumActiveDocs; }
    void updateActiveDocs(uint64_t newVal, uint64_t oldVal) {
        uint64_t tmp = _sumActiveDocs + newVal - oldVal;
        _sumActiveDocs = tmp;
    }
private:
    const uint64_t _decayRate;
    double   _avgSearchTime;
    uint64_t _sumActiveDocs;
    uint64_t _numQueries;
};

/**
 * StateOfRows keeps track of the state of all rows/groups.
 * Currently used for tracking latency in groups. This latency
 * can be used for selecting a random node with weighted probability
 * with the intention to favor load on fast groups.
 **/
class StateOfRows {
public:
    StateOfRows(size_t numRows, double initial, uint64_t decayRate);
    void updateSearchTime(double searchTime, uint32_t rowId);
    const RowState & getRowState(uint32_t rowId) const { return _rows[rowId]; }
    RowState & getRowState(uint32_t rowId) { return _rows[rowId]; }
    uint32_t getRandomWeightedRow() const;
    uint32_t getWeightedNode(double rnd) const;
    void updateActiveDocs(uint32_t rowId, PossCount newVal, PossCount oldVal);
    uint32_t numRowStates() const { return _rows.size(); }
    uint64_t sumActiveDocs() const { return _sumActiveDocs; }
    PossCount getActiveDocs() const;
    bool activeDocsValid() const { return _invalidActiveDocsCounter == 0; }
private:
    std::vector<RowState> _rows;
    uint64_t              _sumActiveDocs;
    size_t                _invalidActiveDocsCounter;
};

}
