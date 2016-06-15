// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Copyright (C) 2001-2003 Fast Search & Transfer ASA
// Copyright (C) 2003 Overture Services Norway AS

#pragma once

#include <vespa/fastos/fastos.h>
#include <vespa/searchlib/parsequery/stackdumpiterator.h>

class StackDumpIteratorTest : public FastOS_Application
{
    int Main();
    void Usage(char *progname);
    bool ShowResult(int testNo, search::SimpleQueryStackDumpIterator &actual, search::SimpleQueryStack &correct, unsigned int expected);
    bool RunTest(int i, bool verify);
};

