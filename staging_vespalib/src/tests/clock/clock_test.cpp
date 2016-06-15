// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP("sharedptr_test");
#include <vespa/vespalib/util/clock.h>
#include <vespa/vespalib/testkit/testapp.h>

using vespalib::Clock;
using fastos::TimeStamp;

class Test : public vespalib::TestApp
{
public:
    int Main();
};


int
Test::Main()
{
    TEST_INIT("clock_test");

    Clock clock(0.050);
    FastOS_ThreadPool pool(0x10000);
    ASSERT_TRUE(pool.NewThread(&clock, NULL) != NULL);
    uint64_t start = clock.getTimeNS();
    FastOS_Thread::Sleep(5000);
    uint64_t stop = clock.getTimeNS();
    EXPECT_TRUE(stop > start);
    FastOS_Thread::Sleep(6000);
    clock.stop();
    uint64_t stop2 = clock.getTimeNS();
    EXPECT_TRUE(stop2 > stop);
    EXPECT_TRUE((stop2 - stop)/TimeStamp::MICRO > 1000);
    TEST_DONE();
}

TEST_APPHOOK(Test)
