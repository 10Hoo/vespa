// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.server.maintenance;

import com.yahoo.cloud.config.ConfigserverConfig;
import com.yahoo.component.AbstractComponent;
import com.yahoo.vespa.config.server.ApplicationRepository;
import com.yahoo.vespa.curator.Curator;

import java.time.Duration;

public class ConfigServerMaintenance extends AbstractComponent {

    private final TenantsMaintainer tenantsMaintainer;

    @SuppressWarnings("unused") // instantiated by Dependency Injection
    public ConfigServerMaintenance(ConfigserverConfig configserverConfig,
                                   ApplicationRepository applicationRepository,
                                   Curator curator) {
        tenantsMaintainer = new TenantsMaintainer(applicationRepository,
                                                  curator,
                                                  Duration.ofMinutes(configserverConfig.tenantsMaintainerIntervalMinutes()));
    }

    @Override
    public void deconstruct() {
        tenantsMaintainer.deconstruct();
    }

}
