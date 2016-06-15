// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Unit tests for autoclosurecaller.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP("autoclosurecaller_test");

#include <vespa/vespalib/testkit/testapp.h>
#include <vespa/vespalib/util/autoclosurecaller.h>

using namespace vespalib;

namespace {

class Test : public vespalib::TestApp {
    void requireThatClosureIsCalledInDtor();

public:
    int Main();
};

int
Test::Main()
{
    TEST_INIT("autoclosurecaller_test");

    TEST_DO(requireThatClosureIsCalledInDtor());

    TEST_DONE();
}

void setBool(bool *b) {
    *b = true;
}

void Test::requireThatClosureIsCalledInDtor() {
    bool is_called = false;
    {
        AutoClosureCaller caller(makeClosure(setBool, &is_called));
        EXPECT_TRUE(!is_called);
    }
    EXPECT_TRUE(is_called);
}

}  // namespace

TEST_APPHOOK(Test);
