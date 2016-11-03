// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "testapp.h"

namespace vespalib {

TestMaster &TestApp::master(TestMaster::master);

TestApp::TestApp()
    : FastOS_Application(),
      _name("<unnamed>")
{
}

TestApp::~TestApp()
{
}

void
TestApp::ReportInit(const char *name)
{
    _name = name;
}

bool
TestApp::ReportConclusion()
{
    return true;
}

} // namespace vespalib
