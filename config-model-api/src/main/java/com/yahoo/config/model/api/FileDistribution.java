// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.model.api;

import com.yahoo.config.FileReference;
import com.yahoo.vespa.defaults.Defaults;

import java.io.File;
import java.util.Collection;
import java.util.Set;

/**
 * Interface for models towards filedistribution.
 *
 * @author lulf
 * @since 5.1
 */
public interface FileDistribution {

    void sendDeployedFiles(String hostName, Set<FileReference> fileReferences);
    void reloadDeployFileDistributor();

    void removeDeploymentsThatHaveDifferentApplicationId(Collection<String> targetHostnames);

    static String getDefaultFileDBRoot() {
        return Defaults.getDefaults().underVespaHome("var/db/vespa/filedistribution");
    }

    static File getDefaultFileDBPath() {
        return new File(getDefaultFileDBRoot());
    }

}
