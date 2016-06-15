// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

namespace vbench {

/**
 * Simple utility class used to handle low-level time sampling.
 **/
class Timer
{
private:
    FastOS_Time _time;

public:
    Timer();
    void reset();
    double sample() const;
};

} // namespace vbench

