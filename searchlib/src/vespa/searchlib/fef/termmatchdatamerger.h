// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "termfieldmatchdataarray.h"
#include "termfieldmatchdata.h"
#include <vector>

namespace search {
namespace fef {

class TermMatchDataMerger
{
public:
    struct Input {
        const TermFieldMatchData *matchData;
        double exactness;

        Input() : matchData(NULL), exactness(0.0) {}
        Input(const TermFieldMatchData *arg_matchData, double arg_exactness)
            : matchData(arg_matchData), exactness(arg_exactness)
        {}
    };
    typedef std::vector<Input> Inputs;
private:
    std::vector<Inputs>                     _inputs;
    const TermFieldMatchDataArray           _output;
    std::vector<TermFieldMatchDataPosition> _scratch;

    TermMatchDataMerger(const TermMatchDataMerger &);
    TermMatchDataMerger &operator=(const TermMatchDataMerger &);

    void merge(uint32_t docid,
               const Inputs &in,
               TermFieldMatchData &out);
public:

    TermMatchDataMerger(const Inputs &allinputs,
                        const TermFieldMatchDataArray &outputs);

    void merge(uint32_t docid);
};

} // namespace fef
} // namespace search

