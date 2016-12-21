// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.


#include "socket_address.h"
#include <vespa/vespalib/util/stringfmt.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cassert>

namespace vespalib {

vespalib::string
SocketAddress::ip_address() const
{
    vespalib::string result = "invalid";
    if (is_ipv4()) {
        char buf[INET_ADDRSTRLEN];
        const sockaddr_in *addr = reinterpret_cast<const sockaddr_in *>(&_addr);
        result = inet_ntop(AF_INET, &addr->sin_addr, buf, sizeof(buf));
    } else if (is_ipv6()) {
        char buf[INET6_ADDRSTRLEN];
        const sockaddr_in6 *addr = reinterpret_cast<const sockaddr_in6 *>(&_addr);
        result = inet_ntop(AF_INET6, &addr->sin6_addr, buf, sizeof(buf));
    }
    return result;
}

vespalib::string
SocketAddress::path() const
{
    vespalib::string result = "";
    if (is_ipc()) {
        const sockaddr_un *addr = reinterpret_cast<const sockaddr_un *>(&_addr);
        const char *path_limit = (reinterpret_cast<const char *>(addr) + _size);
        const char *pos = &addr->sun_path[0];
        const char *end = pos;
        while ((end < path_limit) && (*end != 0)) {
            ++end;
        }
        result.assign(pos, end - pos);
    }
    return result;
}

SocketAddress::SocketAddress(const sockaddr *addr_in, socklen_t addrlen_in)
    : _size(addrlen_in),
      _addr()
{
    memset(&_addr, 0, sizeof(_addr));
    memcpy(&_addr, addr_in, _size);
}

int
SocketAddress::port() const
{
    if (is_ipv4()) {
        const sockaddr_in *addr = reinterpret_cast<const sockaddr_in *>(&_addr);
        return ntohs(addr->sin_port);
    }
    if (is_ipv6()) {
        const sockaddr_in6 *addr = reinterpret_cast<const sockaddr_in6 *>(&_addr);
        return ntohs(addr->sin6_port);        
    }
    return -1;
}

vespalib::string
SocketAddress::spec() const
{
    if (is_ipv4()) {
        return make_string("tcp/%s:%d", ip_address().c_str(), port());
    }
    if (is_ipv6()) {
        return make_string("tcp/[%s]:%d", ip_address().c_str(), port());
    }
    if (is_ipc()) {
        return make_string("ipc/file:%s", path().c_str());
    }
    return "invalid";
}

SocketHandle
SocketAddress::connect() const
{
    if (valid()) {
        SocketHandle handle(socket(_addr.ss_family, SOCK_STREAM, 0));
        if (handle && (::connect(handle.get(), addr(), _size) == 0)) {
            return handle;
        }
    }
    return SocketHandle();
}

SocketHandle
SocketAddress::listen(int backlog) const
{
    if (valid()) {
        SocketHandle handle(socket(_addr.ss_family, SOCK_STREAM, 0));
        if (handle) {
            if (is_ipv6()) {
                int disable = 0;
                setsockopt(handle.get(), IPPROTO_IPV6, IPV6_V6ONLY, &disable, sizeof(disable));
            }
            if (port() > 0) {
                int enable = 1;
                setsockopt(handle.get(), SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
            }
            if ((bind(handle.get(), addr(), _size) == 0) &&
                (::listen(handle.get(), backlog) == 0))
            {
                return handle;
            }
        }
    }
    return SocketHandle();
}

SocketAddress
SocketAddress::address_of(int sockfd)
{
    SocketAddress result;
    sockaddr *addr = reinterpret_cast<sockaddr *>(&result._addr);
    socklen_t addr_len = sizeof(result._addr);
    if (getsockname(sockfd, addr, &addr_len) == 0) {
        assert(addr_len <= sizeof(result._addr));
        result._size = addr_len;
    }
    return result;
}

SocketAddress
SocketAddress::peer_address(int sockfd)
{
    SocketAddress result;
    sockaddr *addr = reinterpret_cast<sockaddr *>(&result._addr);
    socklen_t addr_len = sizeof(result._addr);
    if (getpeername(sockfd, addr, &addr_len) == 0) {
        assert(addr_len <= sizeof(result._addr));
        result._size = addr_len;
    }
    return result;
}

std::vector<SocketAddress>
SocketAddress::resolve(int port, const char *node) {
    std::vector<SocketAddress> result;
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = (AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG);
    vespalib::string service = make_string("%d", port);
    addrinfo *list = nullptr;
    if (getaddrinfo(node, service.c_str(), &hints, &list) == 0) {
        for (const addrinfo *info = list; info != nullptr; info = info->ai_next) {
            result.push_back(SocketAddress(info->ai_addr, info->ai_addrlen));
        }
        freeaddrinfo(list);
    }
    return result;
}

SocketAddress
SocketAddress::select_local(int port, const char *node)
{
    auto prefer_ipv6 = [](const auto &a, const auto &b) { return (!a.is_ipv6() && b.is_ipv6()); };
    return select(prefer_ipv6, port, node);
}

SocketAddress
SocketAddress::select_remote(int port, const char *node)
{
    auto prefer_ipv4 = [](const auto &a, const auto &b) { return (!a.is_ipv4() && b.is_ipv4()); };
    return select(prefer_ipv4, port, node);
}

SocketAddress
SocketAddress::from_path(const vespalib::string &path)
{
    SocketAddress result;
    sockaddr_un &addr_un = reinterpret_cast<sockaddr_un &>(result._addr);
    assert(sizeof(sockaddr_un) <= sizeof(result._addr));
    if (path.size() < sizeof(addr_un.sun_path)) {
        addr_un.sun_family = AF_UNIX;
        strcpy(&addr_un.sun_path[0], path.c_str());        
        result._size = sizeof(sockaddr_un);
    }
    return result;
}

} // namespace vespalib
