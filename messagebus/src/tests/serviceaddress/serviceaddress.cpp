// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP("serviceaddress_test");

#include <vespa/vespalib/testkit/testapp.h>
#include <vespa/vespalib/util/vstringfmt.h>
#include <vespa/messagebus/messagebus.h>
#include <vespa/messagebus/sourcesession.h>
#include <vespa/messagebus/intermediatesession.h>
#include <vespa/messagebus/destinationsession.h>
#include <vespa/messagebus/emptyreply.h>
#include <vespa/messagebus/error.h>
#include <vespa/messagebus/errorcode.h>
#include <vespa/messagebus/routing/routingspec.h>
#include <vespa/messagebus/network/rpcservice.h>
#include <vespa/messagebus/sourcesessionparams.h>
#include <vespa/messagebus/testlib/slobrok.h>
#include <vespa/messagebus/testlib/testserver.h>
#include <vespa/messagebus/testlib/receptor.h>
#include <vespa/messagebus/testlib/simplemessage.h>
#include <vespa/messagebus/testlib/simplereply.h>
#include <vespa/messagebus/testlib/simpleprotocol.h>

using namespace mbus;

class Test : public vespalib::TestApp {
public:
    int Main();
    void testAddrServiceAddress();
    void testNameServiceAddress();

private:
    bool waitSlobrok(RPCNetwork &network, const string &pattern, size_t num);
    bool testAddress(RPCNetwork& network, const string &pattern,
                     const string &expectedSpec, const string &expectedSession);
    bool testNullAddress(RPCNetwork &network, const string &pattern);
};

int
Test::Main()
{
    TEST_INIT("serviceaddress_test");

    testAddrServiceAddress(); TEST_FLUSH();
    testNameServiceAddress(); TEST_FLUSH();

    TEST_DONE();
}

TEST_APPHOOK(Test);

void
Test::testAddrServiceAddress()
{
    Slobrok slobrok;
    RPCNetwork network(RPCNetworkParams()
                       .setIdentity(Identity("foo"))
                       .setSlobrokConfig(slobrok.config()));
    ASSERT_TRUE(network.start());

    EXPECT_TRUE(testNullAddress(network, "tcp"));
    EXPECT_TRUE(testNullAddress(network, "tcp/"));
    EXPECT_TRUE(testNullAddress(network, "tcp/localhost"));
    EXPECT_TRUE(testNullAddress(network, "tcp/localhost:"));
    EXPECT_TRUE(testNullAddress(network, "tcp/localhost:1977"));
    EXPECT_TRUE(testNullAddress(network, "tcp/localhost:1977/"));
    EXPECT_TRUE(testAddress(network, "tcp/localhost:1977/session", "tcp/localhost:1977", "session"));
    EXPECT_TRUE(testNullAddress(network, "tcp/localhost:/session"));
    EXPECT_TRUE(testNullAddress(network, "tcp/:1977/session"));
    EXPECT_TRUE(testNullAddress(network, "tcp/:/session"));

    network.shutdown();
}

void
Test::testNameServiceAddress()
{
    Slobrok slobrok;
    RPCNetwork network(RPCNetworkParams()
                       .setIdentity(Identity("foo"))
                       .setSlobrokConfig(slobrok.config()));
    ASSERT_TRUE(network.start());

    network.unregisterSession("session");
    ASSERT_TRUE(waitSlobrok(network, "foo/session", 0));
    EXPECT_TRUE(testNullAddress(network, "foo/session"));

    network.registerSession("session");
    ASSERT_TRUE(waitSlobrok(network, "foo/session", 1));
    EXPECT_TRUE(testAddress(network, "foo/session", network.getConnectionSpec().c_str(), "session"));

    network.shutdown();
}

bool
Test::waitSlobrok(RPCNetwork &network, const string &pattern, size_t num)
{
    for (int i = 0; i < 1000; i++) {
        slobrok::api::IMirrorAPI::SpecList res = network.getMirror().lookup(pattern);
        if (res.size() == num) {
            return true;
        }
        FastOS_Thread::Sleep(10);
    }
    return false;
}

bool
Test::testNullAddress(RPCNetwork &network, const string &pattern)
{
    RPCService service(network.getMirror(), pattern);
    RPCServiceAddress::UP obj = service.resolve();
    if (!EXPECT_TRUE(obj.get() == NULL)) {
        return false;
    }
    return true;
}

bool
Test::testAddress(RPCNetwork &network, const string &pattern,
                    const string &expectedSpec, const string &expectedSession)
{
    RPCService service(network.getMirror(), pattern);
    RPCServiceAddress::UP obj = service.resolve();
    if (!EXPECT_TRUE(obj.get() != NULL)) {
        return false;
    }
    if (!EXPECT_EQUAL(expectedSpec, obj->getConnectionSpec())) {
        return false;
    }
    if (!EXPECT_EQUAL(expectedSession, obj->getSessionName())) {
        return false;
    }
    return true;
}

