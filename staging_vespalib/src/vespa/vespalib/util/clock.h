// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/fastos/fastos.h>
#include <vespa/fastos/timestamp.h>

namespace vespalib {

/**
 * Clock is a clock that updates the time at defined intervals.
 * It is intended used where you want to check the time with low cost, but where
 * resolution is not that important.
 */

class Clock : public FastOS_Runnable
{
private:
    Clock(const Clock &);
    Clock & operator = (const Clock &);

    mutable fastos::TimeStamp _timeNS;
    int               _timePeriodMS;
    FastOS_Cond       _cond;
    bool              _stop;
    bool              _running;

    void setTime() const;

    virtual void Run(FastOS_ThreadInterface *thisThread, void *arguments);

public:
    Clock(double timePeriod=0.100);
    ~Clock();

    fastos::TimeStamp getTimeNS(void) const {
        if (!_running) {
            setTime();
        }
        return _timeNS;
    }
    fastos::TimeStamp getTimeNSAssumeRunning() const { return _timeNS; }

    void stop(void);
};

}

