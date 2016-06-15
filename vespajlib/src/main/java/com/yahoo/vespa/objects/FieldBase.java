// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.objects;

/**
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 */
public class FieldBase {
    private final String name;

    public FieldBase(String name) {
        this.name = name;
    }

    public final String getName() {
        return name;
    }

    @Override
    public boolean equals(Object o) {
        return this == o || o instanceof FieldBase && name.equalsIgnoreCase(((FieldBase) o).name);
    }

    @Override
    public int hashCode() {
        return name.toLowerCase(java.util.Locale.US).hashCode();
    }

    public String toString() {
        return "field " + name;
    }
}
