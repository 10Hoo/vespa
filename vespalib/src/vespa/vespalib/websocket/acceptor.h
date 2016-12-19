// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "handler.h"
#include "server_socket.h"
#include <thread>

namespace vespalib {
namespace ws {

class Acceptor {
private:
    ServerSocket _server_socket;
    std::thread _accept_thread;

    void accept_main(Handler<Socket> &socket_handler);

public:
    Acceptor(int port_in, Handler<Socket> &socket_handler);
    ~Acceptor();
    int port() { return _server_socket.port(); }
};

} // namespace vespalib::ws
} // namespace vespalib
