// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "transport_security_options.h"
#include <openssl/crypto.h>
#include <cassert>

namespace vespalib::net::tls {

TransportSecurityOptions::TransportSecurityOptions(Builder builder)
    : _ca_certs_pem(std::move(builder._ca_certs_pem)),
      _cert_chain_pem(std::move(builder._cert_chain_pem)),
      _private_key_pem(std::move(builder._private_key_pem)),
      _authorized_peers(std::move(builder._authorized_peers))
{
}

TransportSecurityOptions::TransportSecurityOptions(vespalib::string ca_certs_pem,
                                                   vespalib::string cert_chain_pem,
                                                   vespalib::string private_key_pem)
    : _ca_certs_pem(std::move(ca_certs_pem)),
      _cert_chain_pem(std::move(cert_chain_pem)),
      _private_key_pem(std::move(private_key_pem)),
      _authorized_peers(AuthorizedPeers::allow_all_authenticated())
{
}

TransportSecurityOptions::TransportSecurityOptions(vespalib::string ca_certs_pem,
                                                   vespalib::string cert_chain_pem,
                                                   vespalib::string private_key_pem,
                                                   AuthorizedPeers authorized_peers)
    : _ca_certs_pem(std::move(ca_certs_pem)),
      _cert_chain_pem(std::move(cert_chain_pem)),
      _private_key_pem(std::move(private_key_pem)),
      _authorized_peers(std::move(authorized_peers))
{
}

TransportSecurityOptions::~TransportSecurityOptions() {
    OPENSSL_cleanse(&_private_key_pem[0], _private_key_pem.size());
}

} // vespalib::net::tls
