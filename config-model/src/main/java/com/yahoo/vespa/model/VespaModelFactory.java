// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model;

import com.google.inject.Inject;
import com.yahoo.component.provider.ComponentRegistry;
import com.yahoo.config.application.api.ApplicationPackage;
import com.yahoo.config.application.api.DeployLogger;
import com.yahoo.config.model.ConfigModelRegistry;
import com.yahoo.config.model.MapConfigModelRegistry;
import com.yahoo.config.model.NullConfigModelRegistry;
import com.yahoo.config.model.api.ConfigChangeAction;
import com.yahoo.config.model.api.ConfigModelPlugin;
import com.yahoo.config.model.api.HostProvisioner;
import com.yahoo.config.model.api.Model;
import com.yahoo.config.model.api.ModelContext;
import com.yahoo.config.model.api.ModelCreateResult;
import com.yahoo.config.model.api.ModelFactory;
import com.yahoo.config.model.application.provider.ApplicationPackageXmlFilesValidator;
import com.yahoo.config.model.builder.xml.ConfigModelBuilder;
import com.yahoo.config.model.deploy.DeployProperties;
import com.yahoo.config.model.deploy.DeployState;
import com.yahoo.config.model.provision.HostsXmlProvisioner;
import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.Version;
import com.yahoo.config.provision.Zone;
import com.yahoo.vespa.config.VespaVersion;
import com.yahoo.vespa.model.application.validation.Validation;

import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.Reader;
import java.time.Clock;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

/**
 * Factory for creating {@link VespaModel} instances.
 *
 * @author lulf
 */
public class VespaModelFactory implements ModelFactory {

    private static final Logger log = Logger.getLogger(VespaModelFactory.class.getName());
    private final ConfigModelRegistry configModelRegistry;
    private final Zone zone;
    private final Clock clock;

    @Inject
    public VespaModelFactory(ComponentRegistry<ConfigModelPlugin> pluginRegistry, Zone zone) {
        List<ConfigModelBuilder> modelBuilders = new ArrayList<>();
        for (ConfigModelPlugin plugin : pluginRegistry.allComponents()) {
            if (plugin instanceof ConfigModelBuilder) {
                modelBuilders.add((ConfigModelBuilder) plugin);
            }
        }
        this.configModelRegistry = new MapConfigModelRegistry(modelBuilders);
        this.zone = zone;
        this.clock = Clock.systemUTC();
    }

    public VespaModelFactory(ConfigModelRegistry configModelRegistry) {
        this(configModelRegistry, Clock.systemUTC());
    }
    public VespaModelFactory(ConfigModelRegistry configModelRegistry, Clock clock) {
        if (configModelRegistry == null) {
            this.configModelRegistry = new NullConfigModelRegistry();
            log.info("Will not load config models from plugins, as no registry is available");
        } else {
            this.configModelRegistry = configModelRegistry;
        }
        this.zone = Zone.defaultZone();
        this.clock = clock;
    }

    @Override
    public Version getVersion() {
        return Version.fromIntValues(VespaVersion.major, VespaVersion.minor, VespaVersion.micro);
    }

    @Override
    public Model createModel(ModelContext modelContext) {
        return buildModel(createDeployState(modelContext));
    }

    @Override
    public ModelCreateResult createAndValidateModel(ModelContext modelContext, boolean ignoreValidationErrors) {
        if (modelContext.appDir().isPresent()) {
            ApplicationPackageXmlFilesValidator validator =
                    ApplicationPackageXmlFilesValidator.createDefaultXMLValidator(modelContext.appDir().get(),
                                                                                  modelContext.deployLogger(),
                                                                                  modelContext.vespaVersion());
            try {
                validator.checkApplication();
                ApplicationPackageXmlFilesValidator.checkIncludedDirs(modelContext.applicationPackage());
            } catch (IllegalArgumentException e) {
                rethrowUnlessIgnoreErrors(e, ignoreValidationErrors);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }

        } else {
            validateXML(modelContext.applicationPackage(), modelContext.deployLogger(), ignoreValidationErrors);
        }
        DeployState deployState = createDeployState(modelContext);
        VespaModel model = buildModel(deployState);
        List<ConfigChangeAction> changeActions = validateModel(model, deployState, ignoreValidationErrors);
        return new ModelCreateResult(model, changeActions);
    }

    private VespaModel buildModel(DeployState deployState) {
        try {
            return new VespaModel(configModelRegistry, deployState);
        } catch (IOException | SAXException e) {
            throw new RuntimeException(e);
        }
    }

    private DeployState createDeployState(ModelContext modelContext) {
        DeployState.Builder builder = new DeployState.Builder()
            .applicationPackage(modelContext.applicationPackage())
            .deployLogger(modelContext.deployLogger())
            .configDefinitionRepo(modelContext.configDefinitionRepo())
            .fileRegistry(modelContext.getFileRegistry())
            .permanentApplicationPackage(modelContext.permanentApplicationPackage())
            .properties(createDeployProperties(modelContext.properties()))
            .modelHostProvisioner(createHostProvisioner(modelContext))
            .rotations(modelContext.properties().rotations())
            .zone(zone)
            .now(clock.instant());
        modelContext.previousModel().ifPresent(builder::previousModel);
        return builder.build();
    }

    private DeployProperties createDeployProperties(ModelContext.Properties properties) {
        return new DeployProperties.Builder()
                .applicationId(properties.applicationId())
                .configServerSpecs(properties.configServerSpecs())
                .multitenant(properties.multitenant())
                .hostedVespa(properties.hostedVespa())
                .vespaVersion(getVersion())
                .zone(properties.zone())
                .build();
    }


    private static HostProvisioner createHostProvisioner(ModelContext modelContext) {
        if (isHostedVespaRoutingApplication(modelContext)) {
            //TODO: This belongs in HostedVespaProvisioner.
            //Added here for now since com.yahoo.config.model.api.HostProvisioner is not created per application,
            //and allocation is independent of ApplicationPackage.
            return new HostsXmlProvisioner(hostsXml(modelContext));
        } else {
            return modelContext.hostProvisioner().orElse(
                    DeployState.getDefaultModelHostProvisioner(modelContext.applicationPackage()));
        }
    }

    private static Reader hostsXml(ModelContext modelContext) {
        Reader hosts = modelContext.applicationPackage().getHosts();
        if (hosts == null) {
            //TODO: throw InvalidApplicationException directly. Not possible now, as it resides in the configserver module.
            //SessionPreparer maps IllegalArgumentException -> InvalidApplicationException
            throw new IllegalArgumentException("Hosted vespa routing application must use " + ApplicationPackage.HOSTS
                                               + " to allocate hosts.");
        }
        return hosts;
    }

    private static boolean isHostedVespaRoutingApplication(ModelContext modelContext) {
        ApplicationId id = modelContext.properties().applicationId();
        return modelContext.properties().hostedVespa() && id.isHostedVespaRoutingApplication();
    }

    private void validateXML(ApplicationPackage applicationPackage, DeployLogger deployLogger, boolean ignoreValidationErrors) {
        try {
            applicationPackage.validateXML(deployLogger);
        } catch (IllegalArgumentException e) {
            rethrowUnlessIgnoreErrors(e, ignoreValidationErrors);
        } catch (Exception e) {
             throw new RuntimeException(e);
        }
    }

    private List<ConfigChangeAction> validateModel(VespaModel model, DeployState deployState, boolean ignoreValidationErrors) {
        try {
            deployState.getApplicationPackage().validateXML(deployState.getDeployLogger());
            return Validation.validate(model, ignoreValidationErrors, deployState);
        } catch (IllegalArgumentException e) {
            rethrowUnlessIgnoreErrors(e, ignoreValidationErrors);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        return new ArrayList<>();
    }

    private static void rethrowUnlessIgnoreErrors(IllegalArgumentException e, boolean ignoreValidationErrors) {
        if (!ignoreValidationErrors) {
            throw e;
        }
    }

}
