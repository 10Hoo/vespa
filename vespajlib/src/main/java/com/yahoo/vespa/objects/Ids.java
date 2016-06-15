// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.objects;

/**
 * This is a class containing the global ids that are given out.
 * Must be in sync with version for c++ in staging_vespalib/src/vespalib/objects/ids.h
 *
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 */
public interface Ids {
    public static int document = 0x1000;
    public static int searchlib = 0x4000;
    public static int vespa_configmodel = 0x7000;
    public static int annotation = 0x10000;
}
