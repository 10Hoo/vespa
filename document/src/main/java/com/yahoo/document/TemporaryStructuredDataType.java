// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document;

/**
 * Internal class, DO NOT USE!!&nbsp;Only public because it must be used from com.yahoo.searchdefinition.parser.
 *
 * @author <a href="mailto:einarmr@yahoo-inc.com">Einar M R Rosenvinge</a>
 */
public class TemporaryStructuredDataType extends StructDataType {
    TemporaryStructuredDataType(String name) {
        super(name);
    }

    public static TemporaryStructuredDataType create(String name) {
        return new TemporaryStructuredDataType(name);
    }

    @Override
    protected void setName(String name) {
        super.setName(name);
        setId(createId(getName()));
    }
}
