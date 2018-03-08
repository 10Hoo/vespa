// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.athenz.tls;

import com.yahoo.vespa.athenz.api.AthenzIdentityCertificate;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.security.GeneralSecurityException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

/**
 * @author bjorncs
 */
public class AthenzSslContextBuilder {

    public enum KeyStoreType {
        JKS {
            KeyStore createKeystore() throws KeyStoreException {
                return KeyStore.getInstance("JKS");
            }
        },
        PKCS12 {
            private final BouncyCastleProvider bouncyCastleProvider = new BouncyCastleProvider();

            KeyStore createKeystore() throws KeyStoreException {
                return KeyStore.getInstance("PKCS12", bouncyCastleProvider);
            }
        };
        abstract KeyStore createKeystore() throws GeneralSecurityException;
    }

    private KeyStoreSupplier trustStoreSupplier;
    private KeyStoreSupplier keyStoreSupplier;
    private char[] keyStorePassword;

    public AthenzSslContextBuilder() {}

    public AthenzSslContextBuilder withTrustStore(File file, KeyStoreType trustStoreType) {
        this.trustStoreSupplier = () -> loadKeyStoreFromFile(file, null, trustStoreType);
        return this;
    }

    public AthenzSslContextBuilder withTrustStore(KeyStore trustStore) {
        this.trustStoreSupplier = () -> trustStore;
        return this;
    }

    public AthenzSslContextBuilder withIdentityCertificate(AthenzIdentityCertificate certificate) {
        return withKeyStore(certificate.getPrivateKey(), certificate.getCertificate());
    }

    public AthenzSslContextBuilder withKeyStore(PrivateKey privateKey, X509Certificate certificate) {
        char[] pwd = new char[0];
        this.keyStoreSupplier = () -> createJksKeyStore(privateKey, certificate, pwd);
        this.keyStorePassword = pwd;
        return this;
    }

    public AthenzSslContextBuilder withKeyStore(KeyStore keyStore, char[] password) {
        this.keyStoreSupplier = () -> keyStore;
        this.keyStorePassword = password;
        return this;
    }

    public AthenzSslContextBuilder withKeyStore(File file, char[] password, KeyStoreType keyStoreType) {
        this.keyStoreSupplier = () -> loadKeyStoreFromFile(file, password, keyStoreType);
        this.keyStorePassword = password;
        return this;
    }

    public SSLContext build() {
        try {
            SSLContext sslContext = SSLContext.getInstance("TLSv1.2");
            TrustManager[] trustManagers =
                    trustStoreSupplier != null ? createTrustManagers(trustStoreSupplier) : null;
            KeyManager[] keyManagers =
                    keyStoreSupplier != null ? createKeyManagers(keyStoreSupplier, keyStorePassword) : null;
            sslContext.init(keyManagers, trustManagers, null);
            return sslContext;
        } catch (GeneralSecurityException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private static TrustManager[] createTrustManagers(KeyStoreSupplier trustStoreSupplier)
            throws GeneralSecurityException, IOException {
        TrustManagerFactory trustManagerFactory =
                TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(trustStoreSupplier.get());
        return trustManagerFactory.getTrustManagers();
    }

    private static KeyManager[] createKeyManagers(KeyStoreSupplier keyStoreSupplier, char[] password)
            throws GeneralSecurityException, IOException {
        KeyManagerFactory keyManagerFactory =
                KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        keyManagerFactory.init(keyStoreSupplier.get(), password);
        return keyManagerFactory.getKeyManagers();
    }

    private static KeyStore loadKeyStoreFromFile(File file, char[] password, KeyStoreType keyStoreType)
            throws IOException, GeneralSecurityException{
        KeyStore keyStore = keyStoreType.createKeystore();
        try (FileInputStream in = new FileInputStream(file)) {
            keyStore.load(in, password);
        }
        return keyStore;
    }

    private KeyStore createJksKeyStore(PrivateKey privateKey, X509Certificate certificate, char[] password)
            throws GeneralSecurityException, IOException{
        KeyStore keyStore = KeyStoreType.JKS.createKeystore();
        keyStore.load(null);
        keyStore.setKeyEntry("athenz-identity", privateKey, password, new Certificate[]{certificate});
        return keyStore;
    }

    private interface KeyStoreSupplier {
        KeyStore get() throws IOException, GeneralSecurityException;
    }

}
