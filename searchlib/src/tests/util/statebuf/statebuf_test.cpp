// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/log/log.h>
LOG_SETUP("statebuf_test");
#include <vespa/vespalib/testkit/testapp.h>
#include <vespa/vespalib/stllike/string.h>
#include <vespa/searchlib/util/statebuf.h>
#include <string>

namespace search
{

namespace
{


}

class Fixture : public StateBuf
{
    char _buf[1024];

public:
    Fixture()
        : StateBuf(_buf, sizeof(_buf))
    {
    }
};

TEST_F("single character can be appended to stream", Fixture)
{
    f << 'H' << 'e' << 'l' << 'l' << 'o';
    EXPECT_EQUAL("Hello", f.str());
}


TEST_F("strings can be appended to stream", Fixture)
{
    f << "Hello world";
    EXPECT_EQUAL("Hello world", f.str());
}

TEST_F("quoted strings can be appended to stream", Fixture)
{
    f.appendQuoted("This is a quoting test, \\ \" \n oops");
    EXPECT_EQUAL("\"This is a quoting test, \\\\ \\\" \\n oops\"", f.str());
}

TEST_F("keys can be appended to stream", Fixture)
{
    (f.appendKey("foo") << "fooval").appendKey("bar") << "barval";
    EXPECT_EQUAL("foo=fooval bar=barval", f.str());
}


TEST_F("integers can be appended to stream", Fixture)
{
    f << (UINT64_C(1) << 63) << " " << -42l << " " << 0l;
    EXPECT_EQUAL("9223372036854775808 -42 0", f.str());
}

TEST_F("struct timespec can be appended to stream", Fixture)
{
    struct timespec ts;
    ts.tv_sec = 15;
    ts.tv_nsec = 256;
    f << ts;
    EXPECT_EQUAL("15.000000256", f.str());
}

TEST_F("timestamp can be appended to stream", Fixture)
{
    struct timespec ts;
    ts.tv_sec = 16;
    ts.tv_nsec = 257;
    f.appendTimestamp(ts);
    EXPECT_EQUAL("ts=16.000000257", f.str());
}


TEST_F("hexadecimal numbers can be appended to stream", Fixture)
{
    (f.appendHex(0xdeadbeefcafebabeul) << " ").appendHex(0x123456789abcdef0ul);
    EXPECT_EQUAL("0xdeadbeefcafebabe 0x123456789abcdef0", f.str());

}

TEST_F("pointer address can be appended to stream", Fixture)
{
    f.appendAddr(nullptr);
    f.appendAddr(reinterpret_cast<void *>(0x12345ul));
    EXPECT_EQUAL("addr=0x0000000000000000 addr=0x0000000000012345", f.str());
}


TEST_F("base and size methods can be called on stream", Fixture)
{
    f << "Hello world\n";
    std::string s(f.base(), f.base() + f.size());
    EXPECT_EQUAL("Hello world\n", s);
}

}


TEST_MAIN()
{
    TEST_RUN_ALL();
}
