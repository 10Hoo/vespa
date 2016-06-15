// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchdefinition.derived;

/**
 * Classes exportable to configurations
 *
 * @author  <a href="mailto:bratseth@overture.com">bratseth</a>
 */
public interface Exportable {

    /**
     * Exports the configuration of this object
     *
     *
     * @param  toDirectory the directory to export to, does not write to disk if null
     * @throws java.io.IOException if exporting fails, some files may still be created
     */
    public void export(String toDirectory) throws java.io.IOException;

    /**
     * The (short) name of the exported file
     * @return a String with the (short) name of the exported file
     */
    public String getFileName();

}
