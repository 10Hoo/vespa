// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/storageframework/defaultimplementation/clock/realclock.h>

#include <sys/time.h>

namespace storage {
namespace framework {
namespace defaultimplementation {

MicroSecTime
RealClock::getTimeInMicros() const
{
    struct timeval mytime;
    gettimeofday(&mytime, 0);
    return MicroSecTime(mytime.tv_sec * 1000000llu + mytime.tv_usec);
}

MilliSecTime
RealClock::getTimeInMillis() const
{
    struct timeval mytime;
    gettimeofday(&mytime, 0);
    return MilliSecTime(
            mytime.tv_sec * 1000llu + mytime.tv_usec / 1000);
}

SecondTime
RealClock::getTimeInSeconds() const
{
    struct timeval mytime;
    gettimeofday(&mytime, 0);
    return SecondTime(mytime.tv_sec);
}

} // defaultimplementation
} // framework
} // storage
