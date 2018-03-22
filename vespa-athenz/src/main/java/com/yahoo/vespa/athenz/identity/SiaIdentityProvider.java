// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.athenz.identity;

import com.google.inject.Inject;
import com.yahoo.component.AbstractComponent;
import com.yahoo.container.jdisc.athenz.AthenzIdentityProvider;
import com.yahoo.vespa.athenz.api.AthenzIdentityCertificate;
import com.yahoo.vespa.athenz.api.AthenzService;
import com.yahoo.vespa.athenz.tls.AthenzSslContextBuilder;
import com.yahoo.vespa.athenz.tls.KeyStoreType;
import com.yahoo.vespa.athenz.tls.KeyUtils;
import com.yahoo.vespa.athenz.tls.X509CertificateUtils;

import javax.net.ssl.SSLContext;
import java.io.File;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.time.Duration;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

/**
 * @author mortent
 * @author bjorncs
 */
public class SiaIdentityProvider extends AbstractComponent implements AthenzIdentityProvider {

    private static final Duration REFRESH_INTERVAL = Duration.ofHours(1);

    private final AtomicReference<SSLContext> sslContext = new AtomicReference<SSLContext>();
    private final AthenzService service;
    private final File privateKeyFile;
    private final File certificateFile;
    private final File trustStoreFile;
    private final ScheduledExecutorService scheduler;

    @Inject
    public SiaIdentityProvider(SiaProviderConfig config) {
        this(new AthenzService(config.athenzDomain(), config.athenzService()),
             getPrivateKeyFile(config.keyPathPrefix(), config.athenzDomain(), config.athenzService()),
             getCertificateFile(config.keyPathPrefix(), config.athenzDomain(), config.athenzService()),
             new File(config.trustStorePath()),
             new ScheduledThreadPoolExecutor(1));
    }

    SiaIdentityProvider(AthenzService service,
                        File privateKeyFile,
                        File certificateFile,
                        File trustStoreFile,
                        ScheduledExecutorService scheduler) {
        this.service = service;
        this.privateKeyFile = privateKeyFile;
        this.certificateFile = certificateFile;
        this.trustStoreFile = trustStoreFile;
        this.scheduler = scheduler;
        this.sslContext.set(createIdentitySslContext());
        scheduler.scheduleAtFixedRate(this::reloadSslContext, REFRESH_INTERVAL.toMinutes(), REFRESH_INTERVAL.toMinutes(), TimeUnit.MINUTES);
    }

    @Override
    public String getDomain() {
        return service.getDomain().getName();
    }

    @Override
    public String getService() {
        return service.getName();
    }

    @Override
    public SSLContext getIdentitySslContext() {
        return sslContext.get();
    }

    private SSLContext createIdentitySslContext() {
        try {
            String certPem = new String(Files.readAllBytes(certificateFile.toPath()));
            X509Certificate certificate = X509CertificateUtils.fromPem(certPem);
            String keyPem = new String(Files.readAllBytes(privateKeyFile.toPath()));
            PrivateKey privateKey = KeyUtils.fromPemEncodedPrivateKey(keyPem);
            return new AthenzSslContextBuilder()
                    .withTrustStore(trustStoreFile, KeyStoreType.JKS)
                    .withIdentityCertificate(new AthenzIdentityCertificate(certificate, privateKey))
                    .build();
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private void reloadSslContext() {
        this.sslContext.set(createIdentitySslContext());
    }

    private static File getCertificateFile(String rootPath, String domain, String service) {
        return Paths.get(rootPath, "certs", String.format("%s.%s.cert.pem", domain, service)).toFile();
    }

    private static File getPrivateKeyFile(String rootPath, String domain, String service) {
        return Paths.get(rootPath, "keys", String.format("%s.%s.key.pem", domain, service)).toFile();
    }

    @Override
    public void deconstruct() {
        try {
            scheduler.shutdownNow();
            scheduler.awaitTermination(90, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}
