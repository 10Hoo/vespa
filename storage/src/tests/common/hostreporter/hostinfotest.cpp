// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
#include <vespa/storage/common/hostreporter/hostinfo.h>
#include <vespa/storage/common/hostreporter/hostreporter.h>
#include <vespa/vdstestlib/cppunit/macros.h>
#include <vespa/vespalib/data/slime/slime.h>
#include <vespa/vespalib/util/jsonstream.h>
#include "util.h"

LOG_SETUP(".test.hostinforeporter");

namespace storage {
namespace {
using Object = vespalib::JsonStream::Object;
using End = vespalib::JsonStream::End;
using JsonFormat = vespalib::slime::JsonFormat;
using Memory = vespalib::slime::Memory;

class DummyReporter: public HostReporter {
public:
    void report(vespalib::JsonStream& jsonreport) override {
        jsonreport << "dummy" << Object()  << "foo" << "bar" << End();
    }
};
}

struct HostInfoReporterTest : public CppUnit::TestFixture
{
    void testHostInfoReporter();

    CPPUNIT_TEST_SUITE(HostInfoReporterTest);
    CPPUNIT_TEST(testHostInfoReporter);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(HostInfoReporterTest);

void
HostInfoReporterTest::testHostInfoReporter()
{
    HostInfo hostinfo;
    DummyReporter dummyReporter;
    hostinfo.registerReporter(&dummyReporter);
    vespalib::asciistream json;
    vespalib::JsonStream stream(json, true);

    stream << Object();
    hostinfo.printReport(stream);
    stream << End();

    std::string jsonData = json.str();
    vespalib::Slime slime;
    JsonFormat::decode(Memory(jsonData), slime);
    CPPUNIT_ASSERT(slime.get()["dummy"]["foo"].asString() == "bar");
    CPPUNIT_ASSERT(0 < slime.get()["network"]["lo"]["input"]["packets"].asLong());
    CPPUNIT_ASSERT(1.0 <= slime.get()["cpu"]["context switches"].asDouble());
}
} // storage

