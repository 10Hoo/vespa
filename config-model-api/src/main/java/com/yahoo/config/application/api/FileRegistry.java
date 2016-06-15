// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.application.api;

import java.util.List;
import java.util.Set;

import com.yahoo.config.FileReference;

/**
 * @author tonytv
 */
public interface FileRegistry {

    FileReference addFile(String relativePath);

    /**
     * Returns the name of the host which is the source of the files
     */
    String fileSourceHost();

    Set<String> allRelativePaths();

    List<Entry> export();

    class Entry {
        public final String relativePath;
        public final FileReference reference;

        public Entry(String relativePath, FileReference reference) {
            this.relativePath = relativePath;
            this.reference = reference;
        }
    }

}
