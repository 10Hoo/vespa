// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/vespalib/testkit/test_kit.h>
#include <vespa/vespalib/util/host_name.h>
#include <vespa/vespalib/util/slaveproc.h>

using namespace vespalib;

vespalib::string run_cmd(const vespalib::string &cmd) {
    std::string out;
    ASSERT_TRUE(SlaveProc::run(cmd.c_str(), out));
    return out;
}

TEST("require that host name can be obtained") {
    EXPECT_EQUAL(run_cmd("hostname"), HostName::get());
}

TEST_MAIN_WITH_PROCESS_PROXY() { TEST_RUN_ALL(); }
