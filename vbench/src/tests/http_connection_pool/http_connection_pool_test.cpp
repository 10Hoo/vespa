// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/vespalib/testkit/testapp.h>
#include <vbench/test/all.h>
#include <vespa/vespalib/util/sync.h>

using namespace vbench;
using vespalib::CountDownLatch;

TEST_MT_F("http connection pool", 2, ServerSocket()) {
    if (thread_id == 0) {
        for (; f1.accept().get() != 0; ) {}
    } else {
        Timer timer;
        HttpConnection::UP conn;
        HttpConnectionPool pool(timer);
        conn = pool.getConnection(ServerSpec("localhost", f1.port()));
        EXPECT_TRUE(conn.get() != 0);
        pool.putConnection(std::move(conn));
        EXPECT_TRUE(conn.get() == 0);
        conn = pool.getConnection(ServerSpec("localhost", f1.port()));
        EXPECT_TRUE(conn.get() != 0);
        conn->stream().obtain(1, 1); // trigger eof
        pool.putConnection(std::move(conn));
        EXPECT_TRUE(conn.get() == 0);
        conn = pool.getConnection(ServerSpec("localhost", f1.port()));
        EXPECT_TRUE(conn.get() != 0);
        f1.close();
    }
}

TEST_MT_FFFF("stress http connection pool", 256, ServerSocket(), Timer(), HttpConnectionPool(f2),
             CountDownLatch(num_threads-2))
{
    if (thread_id == 0) {
        for (; f1.accept().get() != 0; ) {}
    } else {
        while (f2.sample() < 5.0) {
            HttpConnection::UP conn = f3.getConnection(ServerSpec("localhost", f1.port()));
            EXPECT_TRUE(conn.get() != 0);
            if (thread_id > (num_threads / 2)) {
                conn->stream().obtain(1, 1); // trigger eof
            }
            f3.putConnection(std::move(conn));
            EXPECT_TRUE(conn.get() == 0);
        }
        if (thread_id == 1) {
            f4.await();
            f1.close();
        } else {
            f4.countDown();
        }
    }
}

TEST_MAIN() { TEST_RUN_ALL(); }
