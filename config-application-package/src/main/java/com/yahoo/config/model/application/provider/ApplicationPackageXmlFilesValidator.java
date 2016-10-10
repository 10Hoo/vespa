// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.model.application.provider;

import com.yahoo.collections.Tuple2;
import com.yahoo.config.application.api.ApplicationPackage;
import com.yahoo.config.application.api.DeployLogger;
import com.yahoo.config.provision.Version;
import com.yahoo.path.Path;
import com.yahoo.io.reader.NamedReader;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.List;
import java.util.Optional;

/**
 * Validation of xml files in application package against RELAX NG schemas.
 *
 * @author hmusum
 */
public class ApplicationPackageXmlFilesValidator {

    private final AppSubDirs appDirs;
    private final Optional<Version> vespaVersion;

    private static final FilenameFilter xmlFilter = new FilenameFilter() {
        @Override
        public boolean accept(File dir, String name) {
            return name.endsWith(".xml");
        }
    };


    public ApplicationPackageXmlFilesValidator(AppSubDirs appDirs, Optional<Version> vespaVersion) {
        this.appDirs = appDirs;
        this.vespaVersion = vespaVersion;
    }

    // TODO: Remove when no version older than 6.33 is used
    public ApplicationPackageXmlFilesValidator(AppSubDirs appDirs, DeployLogger logger, Optional<Version> vespaVersion) {
        this.appDirs = appDirs;
        this.vespaVersion = vespaVersion;
    }

    public static ApplicationPackageXmlFilesValidator createDefaultXMLValidator(File appDir, Optional<Version> vespaVersion) {
        return new ApplicationPackageXmlFilesValidator(new AppSubDirs(appDir), vespaVersion);
    }

    public static ApplicationPackageXmlFilesValidator createTestXmlValidator(File appDir) {
        return new ApplicationPackageXmlFilesValidator(new AppSubDirs(appDir), Optional.<Version>empty());
    }

    @SuppressWarnings("deprecation")
    public void checkApplication() throws IOException {
        validateHostsFile(SchemaValidator.hostsXmlSchemaName);
        validateServicesFile(SchemaValidator.servicesXmlSchemaName);
        // TODO: Disable temporarily, need to get out feature to support ignoring validation errors
        //validateDeploymentFile(SchemaValidator.deploymentXmlSchemaName);

        if (appDirs.searchdefinitions().exists()) {
            if (FilesApplicationPackage.getSearchDefinitionFiles(appDirs.root()).isEmpty()) {
                throw new IllegalArgumentException("Application package in " + appDirs.root() +
                        " must contain at least one search definition (.sd) file when directory searchdefinitions/ exists.");
            }
        }

        validate(appDirs.routingtables, "routing-standalone.rnc");
    }

    // For testing
    public static void checkIncludedDirs(ApplicationPackage app) throws IOException {
        for (String includedDir : app.getUserIncludeDirs()) {
            List<NamedReader> includedFiles = app.getFiles(Path.fromString(includedDir), ".xml", true);
            for (NamedReader file : includedFiles) {
                createSchemaValidator("container-include.rnc", Optional.empty()).validate(file);
            }
        }
    }

    @SuppressWarnings("deprecation")
    private void validateHostsFile(String hostsXmlSchemaName) throws IOException {
        if (appDirs.file(FilesApplicationPackage.HOSTS).exists()) {
            validate(hostsXmlSchemaName, FilesApplicationPackage.HOSTS);
        }
    }

    private void validateServicesFile(String servicesXmlSchemaName) throws IOException {
        // vespa-services.xml or services.xml. Fallback to vespa-services.xml
        validate(servicesXmlSchemaName, servicesFileName());
    }

    private void validateDeploymentFile(String deploymentXmlSchemaName) throws IOException {
        if (appDirs.file(FilesApplicationPackage.DEPLOYMENT_FILE.getName()).exists()) {
            validate(deploymentXmlSchemaName, FilesApplicationPackage.DEPLOYMENT_FILE.getName());
        }
    }

    private void validate(String schemaName, String xmlFileName) throws IOException {
        createSchemaValidator(schemaName, vespaVersion).validate(appDirs.file(xmlFileName));
    }

    @SuppressWarnings("deprecation")
    private String servicesFileName() {
        String servicesFile = FilesApplicationPackage.SERVICES;
        if (!appDirs.file(servicesFile).exists()) {
            throw new IllegalArgumentException("Application package in " + appDirs.root() +
                                               " must contain " + FilesApplicationPackage.SERVICES);
        }
        return servicesFile;
    }

    private void validate(Tuple2<File, String> directory, String schemaFile) throws IOException {
        if ( ! directory.first.isDirectory()) return;
        validate(directory, createSchemaValidator(schemaFile, vespaVersion));
    }

    private void validate(Tuple2<File, String> directory, SchemaValidator validator) throws IOException {
        File dir = directory.first;
        if ( ! dir.isDirectory()) return;

        String directoryName = directory.second;
        for (File f : dir.listFiles(xmlFilter)) {
            if (f.isDirectory())
                validate(new Tuple2<>(f, directoryName + File.separator + f.getName()),validator);
            else
                validator.validate(f, directoryName + File.separator + f.getName());
        }
    }

    private static SchemaValidator createSchemaValidator(String schemaFile, Optional<Version> vespaVersion) {
        return new SchemaValidator(schemaFile, new BaseDeployLogger(), vespaVersion);
    }

}
