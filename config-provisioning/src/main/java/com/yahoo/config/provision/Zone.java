// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.provision;

import com.google.common.base.Strings;
import com.google.inject.Inject;
import com.yahoo.cloud.config.ConfigserverConfig;

import java.util.Optional;

/**
 * The zone (environment + region) of this runtime.
 * An injected instance of this will return the correct current environment and region.
 * Components can use this to obtain information about which zone they are running in.
 *
 * @author bratseth
 */
public class Zone {

    private final Environment environment;
    private final RegionName region;
    private final FlavorDefaults flavorDefaults;

    @Inject
    public Zone(ConfigserverConfig configserverConfig) {
        this(Environment.from(configserverConfig.environment()), RegionName.from(configserverConfig.region()),
             new FlavorDefaults(configserverConfig));
    }

    /** Create from environment and region */
    public Zone(Environment environment, RegionName region) { this(environment, region, "default"); }

    /** Create from environment and region. Useful for testing. */
    public Zone(Environment environment, RegionName region, String defaultFlavor) {
        this(environment, region, new FlavorDefaults(defaultFlavor));
    }

    private Zone(Environment environment, RegionName region, FlavorDefaults flavorDefaults) {
        this.environment = environment;
        this.region = region;
        this.flavorDefaults = flavorDefaults;
    }

    /** Returns the current environment */
    public Environment environment() { return environment; }

    /** Returns the current region */
    public RegionName region() { return region; }

    /** Returns the default hardware flavor to assign in this zone */
    public String defaultFlavor(ClusterSpec.Type clusterType) { return flavorDefaults.flavor(clusterType); }

    /** Do not use */
    public static Zone defaultZone() {
        return new Zone(Environment.defaultEnvironment(), RegionName.defaultName());
    }

    @Override
    public String toString() {
        return "zone " + environment + "." + region;
    }

    private static class FlavorDefaults {

        /** The default default flavor */
        private final String defaultFlavor;

        /** The default flavor for each cluster type, or empty to use defaultFlavor */
        private final Optional<String> adminFlavor;
        private final Optional<String> containerFlavor;
        private final Optional<String> contentFlavor;

        /** Creates this with a default flavor and all cluster type flavors empty */
        public FlavorDefaults(String defaultFlavor) {
            this(defaultFlavor, Optional.empty(), Optional.empty(), Optional.empty());
        }

        /** Creates this with a default flavor and all cluster type flavors empty */
        public FlavorDefaults(String defaultFlavor,
                              Optional<String> adminFlavor, Optional<String> containerFlavor, Optional<String> contentFlavor) {
            this.defaultFlavor = defaultFlavor;
            this.adminFlavor = adminFlavor;
            this.containerFlavor = containerFlavor;
            this.contentFlavor = contentFlavor;
        }

        public FlavorDefaults(ConfigserverConfig config) {
            this(config.defaultFlavor(),
                 emptyIfDefault(config.defaultAdminFlavor()),
                 emptyIfDefault(config.defaultContainerFlavor()),
                 emptyIfDefault(config.defaultContentFlavor()));
        }

        /** Map "default" to empty - this config cannot have missing values due to the need for supporting non-hosted */
        private static Optional<String> emptyIfDefault(String value) {
            if (Strings.isNullOrEmpty(value)) return Optional.empty();
            if (value.equals("default")) return Optional.empty();
            return Optional.of(value);
        }

        /**
         * Returns the flavor default for a given cluster type.
         * This may be "default" - which is an invalid value - but never null.
         */
        public String flavor(ClusterSpec.Type clusterType) {
            switch (clusterType) {
                case admin: return adminFlavor.orElse(defaultFlavor);
                case container: return containerFlavor.orElse(defaultFlavor);
                case content: return contentFlavor.orElse(defaultFlavor);
                default: return defaultFlavor; // future cluster types
            }
        }

    }

}
