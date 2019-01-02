// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.deployment;

import com.yahoo.config.application.api.DeploymentSpec;
import com.yahoo.config.application.api.ValidationId;
import com.yahoo.config.provision.AthenzDomain;
import com.yahoo.config.provision.AthenzService;
import com.yahoo.config.provision.Environment;
import com.yahoo.vespa.hosted.controller.application.ApplicationPackage;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.time.Duration;
import java.time.Instant;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.Optional;
import java.util.StringJoiner;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * A builder that builds application packages for testing purposes.
 *
 * @author mpolden
 */
public class ApplicationPackageBuilder {

    private final StringBuilder environmentBody = new StringBuilder();
    private final StringBuilder validationOverridesBody = new StringBuilder();
    private final StringBuilder blockChange = new StringBuilder();

    private Optional<Integer> majorVersion = Optional.empty();
    private String upgradePolicy = null;
    private Environment environment = Environment.prod;
    private String globalServiceId = null;
    private String athenzIdentityAttributes = null;
    private String searchDefinition = "search test { }";
    private DeploymentSpec.Notifications notifications = DeploymentSpec.Notifications.none();

    public ApplicationPackageBuilder majorVersion(int majorVersion) {
        this.majorVersion = Optional.of(majorVersion);
        return this;
    }

    public ApplicationPackageBuilder upgradePolicy(String upgradePolicy) {
        this.upgradePolicy = upgradePolicy;
        return this;
    }

    public ApplicationPackageBuilder environment(Environment environment) {
        this.environment = environment;
        return this;
    }

    public ApplicationPackageBuilder globalServiceId(String globalServiceId) {
        this.globalServiceId = globalServiceId;
        return this;
    }

    public ApplicationPackageBuilder region(String regionName) {
        environmentBody.append("    <region active='true'>");
        environmentBody.append(regionName);
        environmentBody.append("</region>\n");
        return this;
    }

    public ApplicationPackageBuilder parallel(String... regionName) {
        environmentBody.append("    <parallel>\n");
        Arrays.stream(regionName).forEach(this::region);
        environmentBody.append("    </parallel>\n");
        return this;
    }

    public ApplicationPackageBuilder delay(Duration delay) {
        environmentBody.append("    <delay seconds='");
        environmentBody.append(delay.getSeconds());
        environmentBody.append("'/>\n");
        return this;
    }

    public ApplicationPackageBuilder blockChange(boolean revision, boolean version, String daySpec, String hourSpec,
                                                 String zoneSpec) {
        blockChange.append("  <block-change");
        blockChange.append(" revision='").append(revision).append("'");
        blockChange.append(" version='").append(version).append("'");
        blockChange.append(" days='").append(daySpec).append("'");
        blockChange.append(" hours='").append(hourSpec).append("'");
        blockChange.append(" time-zone='").append(zoneSpec).append("'");
        blockChange.append("/>\n");
        return this;
    }

    public ApplicationPackageBuilder allow(ValidationId validationId) {
        validationOverridesBody.append("  <allow until='");
        validationOverridesBody.append(asIso8601Date(Instant.now().plus(Duration.ofDays(28))));
        validationOverridesBody.append("'>");
        validationOverridesBody.append(validationId.value());
        validationOverridesBody.append("</allow>\n");
        return this;
    }

    public ApplicationPackageBuilder athenzIdentity(AthenzDomain domain, AthenzService service) {
        this.athenzIdentityAttributes = String.format("athenz-domain='%s' athenz-service='%s'", domain.value(),
                                                      service.value());
        return this;
    }

    public ApplicationPackageBuilder notifyAuthor(boolean notify) {
        this.notifications = DeploymentSpec.Notifications.of(this.notifications.staticEmails(), notify);
        return this;
    }

    public ApplicationPackageBuilder notifyEmails(List<String> emails) {
        this.notifications = DeploymentSpec.Notifications.of(emails, this.notifications.includeAuthor());
        return this;
    }

    /** Sets the content of the search definition test.sd */
    public ApplicationPackageBuilder searchDefinition(String testSearchDefinition) {
        this.searchDefinition = testSearchDefinition;
        return this;
    }
    
    private byte[] deploymentSpec() {
        StringBuilder xml = new StringBuilder();
        xml.append("<deployment version='1.0' ");
        majorVersion.ifPresent(v -> xml.append("major-version='").append(v).append("' "));
        if(athenzIdentityAttributes != null) {
            xml.append(athenzIdentityAttributes);
        }
        xml.append(">\n");
        if (upgradePolicy != null) {
            xml.append("<upgrade policy='");
            xml.append(upgradePolicy);
            xml.append("'/>\n");
        }
        if (notifications != DeploymentSpec.Notifications.none()) {
            xml.append("<notifications>\n");
            if (notifications.includeAuthor())
                xml.append("  <author />\n");
            for (String email : notifications.staticEmails())
                xml.append("  <email>").append(email).append("</email>\n");
            xml.append("</notifications>\n");
        }
        xml.append(blockChange);
        xml.append("  <");
        xml.append(environment.value());
        if (globalServiceId != null) {
            xml.append(" global-service-id='");
            xml.append(globalServiceId);
            xml.append("'");
        }
        xml.append(">\n");
        xml.append(environmentBody);
        xml.append("  </");
        xml.append(environment.value());
        xml.append(">\n</deployment>");
        return xml.toString().getBytes(StandardCharsets.UTF_8);
    }
    
    private byte[] validationOverrides() {
        String xml = "<validation-overrides version='1.0'>\n" +
                validationOverridesBody +
                "</validation-overrides>\n";
        return xml.getBytes(StandardCharsets.UTF_8);
    }

    private byte[] searchDefinition() { 
        return searchDefinition.getBytes(StandardCharsets.UTF_8);
    }

    public ApplicationPackage build() {
        ByteArrayOutputStream zip = new ByteArrayOutputStream();
        try (ZipOutputStream out = new ZipOutputStream(zip)) {
            out.putNextEntry(new ZipEntry("deployment.xml"));
            out.write(deploymentSpec());
            out.closeEntry();
            out.putNextEntry(new ZipEntry("validation-overrides.xml"));
            out.write(validationOverrides());
            out.closeEntry();
            out.putNextEntry(new ZipEntry("search-definitions/test.sd"));
            out.write(searchDefinition());
            out.closeEntry();
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
        return new ApplicationPackage(zip.toByteArray());
    }

    private static String asIso8601Date(Instant instant) {
        return new SimpleDateFormat("yyyy-MM-dd").format(Date.from(instant));
    }

}
