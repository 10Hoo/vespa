// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.system;

public class ForceLoadTestCase extends junit.framework.TestCase {

    public ForceLoadTestCase(String name) {
        super(name);
    }

    public void testLoadClasses() {
        try {
            ForceLoad.forceLoad(getClass().getPackage().getName(), new String[] { "Foo", "Bar" });
        } catch (ForceLoadError e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    public void testLoadBogusClass() {
        try {
            ForceLoad.forceLoad(getClass().getPackage().getName(), new String[] { "Foo", "Bar", "Baz" });
        } catch (ForceLoadError e) {
            return;
        }
        assertTrue(false);
    }
}
