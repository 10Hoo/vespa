// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "socket.h"

class FastOS_UNIX_Socket : public FastOS_SocketInterface
{
public:
    ~FastOS_UNIX_Socket();

    bool Close () override;
    bool Shutdown() override;
    bool SetSoBlocking (bool blockingEnabled) override;
    ssize_t Read (void *readBuffer, size_t bufferSize) override;
    ssize_t Write (const void *writeBuffer, size_t bufferSize) override;

    static int GetLastError () { return errno; }
    static std::string getErrorString(int error);

    enum {
        ERR_ALREADY = EALREADY,               // New style error codes
        ERR_AGAIN = EAGAIN,
        ERR_INTR = EINTR,
        ERR_ISCONN = EISCONN,
        ERR_INPROGRESS = EINPROGRESS,
        ERR_WOULDBLOCK = EWOULDBLOCK,
        ERR_ADDRNOTAVAIL = EADDRNOTAVAIL,
        ERR_MFILE = EMFILE,
        ERR_NFILE = ENFILE,
        ERR_CONNRESET = ECONNRESET,

        ERR_EAGAIN = EAGAIN,                  // Old style error codes
        ERR_EINTR = EINTR,
        ERR_EISCONN = EISCONN,
        ERR_EINPROGRESS = EINPROGRESS,
        ERR_EWOULDBLOCK = EWOULDBLOCK,
        ERR_EADDRNOTAVAIL = EADDRNOTAVAIL,
        ERR_EMFILE = EMFILE,
        ERR_ENFILE = ENFILE
    };
};


