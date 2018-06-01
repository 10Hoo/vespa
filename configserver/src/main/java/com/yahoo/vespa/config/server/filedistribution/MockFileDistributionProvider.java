// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.server.filedistribution;

import com.yahoo.config.model.api.FileDistribution;
import com.yahoo.config.model.application.provider.MockFileRegistry;

import java.io.File;

/**
 * @author Ulf Lilleengen
 */
public class MockFileDistributionProvider extends FileDistributionProvider {
    public int timesCalled = 0;

    public MockFileDistributionProvider(File fileReferencesDir) {
        super(new MockFileRegistry(), new MockFileDistribution(fileReferencesDir));
    }

    public FileDistribution getFileDistribution() {
        timesCalled++;
        return super.getFileDistribution();
    }

}
