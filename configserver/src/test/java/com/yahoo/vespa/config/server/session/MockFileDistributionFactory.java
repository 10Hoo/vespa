// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.server.session;

import com.yahoo.cloud.config.ConfigserverConfig;
import com.yahoo.vespa.config.server.filedistribution.FileDistributionProvider;
import com.yahoo.vespa.config.server.filedistribution.MockFileDistributionProvider;

import java.io.File;

/**
* @author Ulf Lilleengen
*/
public class MockFileDistributionFactory extends FileDistributionFactory {

    public final MockFileDistributionProvider mockFileDistributionProvider;

    public MockFileDistributionFactory(File fileReferencesDir) {
        super(new ConfigserverConfig(new ConfigserverConfig.Builder()));
        mockFileDistributionProvider = new MockFileDistributionProvider(fileReferencesDir);
    }

    @Override
    public FileDistributionProvider createProvider(File applicationFile) {
        return mockFileDistributionProvider;
    }
}
