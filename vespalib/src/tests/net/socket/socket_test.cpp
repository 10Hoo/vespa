// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/vespalib/testkit/test_kit.h>
#include <vespa/vespalib/net/socket_address.h>
#include <vespa/vespalib/net/server_socket.h>
#include <vespa/vespalib/net/socket.h>
#include <vespa/vespalib/util/stringfmt.h>
#include <thread>
#include <functional>
#include <chrono>

using namespace vespalib;

vespalib::string read_bytes(Socket &socket, size_t wanted_bytes) {
    char tmp[64];
    vespalib::string result;
    while (result.size() < wanted_bytes) {
        size_t read_size = std::min(sizeof(tmp), wanted_bytes - result.size());
        size_t read_result = socket.read(tmp, read_size);
        if (read_result <= 0) {
            return result;
        }
        result.append(tmp, read_result);
    }
    return result;
}

void verify_socket_io(bool is_server, Socket &socket) {
    vespalib::string server_message = "hello, this is the server speaking";
    vespalib::string client_message = "please pick up, I need to talk to you";
    if(is_server) {
        socket.write(server_message.data(), server_message.size());
        vespalib::string read = read_bytes(socket, client_message.size());
        EXPECT_EQUAL(client_message, read);
    } else {
        socket.write(client_message.data(), client_message.size());
        vespalib::string read = read_bytes(socket, server_message.size());
        EXPECT_EQUAL(server_message, read);
    }
}

Socket::UP connect_sockets(bool is_server, ServerSocket &server_socket) {
    if (is_server) {
        return server_socket.accept();
    } else {
        return Socket::connect("localhost", server_socket.address().port());
    }
}

//-----------------------------------------------------------------------------

TEST("my local address") {
    auto list = SocketAddress::resolve(4080);
    fprintf(stderr, "resolve(4080):\n");
    for (const auto &addr: list) {
        fprintf(stderr, "  %s\n", addr.spec().c_str());
    }
}

TEST("yahoo.com address") {
    auto list = SocketAddress::resolve(80, "yahoo.com");
    fprintf(stderr, "resolve(80, 'yahoo.com'):\n");
    for (const auto &addr: list) {
        fprintf(stderr, "  %s\n", addr.spec().c_str());
    }
}

struct ServerWrapper {
    ServerSocket::UP server = ServerSocket::listen(0);
};

TEST_MT_F("require that basic socket io works", 2, ServerWrapper) {
    bool is_server = (thread_id == 0);
    Socket::UP socket = connect_sockets(is_server, *f1.server);
    TEST_DO(verify_socket_io(is_server, *socket));
}

TEST_MT_F("require that server accept can be interrupted", 2, ServerWrapper) {
    bool is_server = (thread_id == 0);
    if (is_server) {
        fprintf(stderr, "--> calling accept\n");
        Socket::UP socket = f1.server->accept();
        fprintf(stderr, "<-- accept returned\n");
        EXPECT_TRUE(!socket->valid());
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        fprintf(stderr, "--- closing server socket\n");
        f1.server->shutdown();
    }
}

TEST_MAIN() { TEST_RUN_ALL(); }
