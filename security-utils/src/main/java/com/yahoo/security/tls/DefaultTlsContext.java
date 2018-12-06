// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.security.tls;

import com.yahoo.security.SslContextBuilder;
import com.yahoo.security.tls.authz.PeerAuthorizerTrustManagersFactory;
import com.yahoo.security.tls.policy.AuthorizedPeers;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import java.nio.file.Path;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A static {@link TlsContext}
 *
 * @author bjorncs
 */
public class DefaultTlsContext implements TlsContext {

    public static final List<String> ALLOWED_CIPHER_SUITES = Arrays.asList(
            "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384",
            "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384",
            "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256",
            "TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256",
            "TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256",
            "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256");

    private static final Logger log = Logger.getLogger(DefaultTlsContext.class.getName());

    private final SSLContext sslContext;

    public DefaultTlsContext(List<X509Certificate> certificates,
                             PrivateKey privateKey,
                             List<X509Certificate> caCertificates,
                             AuthorizedPeers authorizedPeers,
                             AuthorizationMode mode) {
        this.sslContext = createSslContext(certificates, privateKey, caCertificates, authorizedPeers, mode);
    }

    public DefaultTlsContext(Path tlsOptionsConfigFile, AuthorizationMode mode) {
        this.sslContext = createSslContext(tlsOptionsConfigFile, mode);
    }

    @Override
    public SSLEngine createSslEngine() {
        SSLEngine sslEngine = sslContext.createSSLEngine();
        restrictSetOfEnabledCiphers(sslEngine);
        return sslEngine;
    }

    private static void restrictSetOfEnabledCiphers(SSLEngine sslEngine) {
        String[] validCipherSuites = Arrays.stream(sslEngine.getSupportedCipherSuites())
                .filter(ALLOWED_CIPHER_SUITES::contains)
                .toArray(String[]::new);
        if (validCipherSuites.length == 0) {
            throw new IllegalStateException("None of the allowed cipher suites are supported");
        }
        log.log(Level.FINE, () -> String.format("Allowed cipher suites that are supported: %s", Arrays.toString(validCipherSuites)));
        sslEngine.setEnabledCipherSuites(validCipherSuites);
    }

    private static SSLContext createSslContext(List<X509Certificate> certificates,
                                               PrivateKey privateKey,
                                               List<X509Certificate> caCertificates,
                                               AuthorizedPeers authorizedPeers,
                                               AuthorizationMode mode) {
        SslContextBuilder builder = new SslContextBuilder();
        if (!certificates.isEmpty()) {
            builder.withKeyStore(privateKey, certificates);
        }
        if (!caCertificates.isEmpty()) {
            builder.withTrustStore(caCertificates);
        }
        if (authorizedPeers != null) {
            builder.withTrustManagerFactory(new PeerAuthorizerTrustManagersFactory(authorizedPeers, mode));
        }
        return builder.build();
    }

    private static SSLContext createSslContext(Path tlsOptionsConfigFile, AuthorizationMode mode) {
        TransportSecurityOptions options = TransportSecurityOptions.fromJsonFile(tlsOptionsConfigFile);
        SslContextBuilder builder = new SslContextBuilder();
        options.getCertificatesFile()
                .ifPresent(certificates -> builder.withKeyStore(options.getPrivateKeyFile().get(), certificates));
        options.getCaCertificatesFile().ifPresent(builder::withTrustStore);
        if (mode != AuthorizationMode.DISABLE) {
            options.getAuthorizedPeers().ifPresent(
                    authorizedPeers -> builder.withTrustManagerFactory(new PeerAuthorizerTrustManagersFactory(authorizedPeers, mode)));
        }
        return builder.build();
    }

}
