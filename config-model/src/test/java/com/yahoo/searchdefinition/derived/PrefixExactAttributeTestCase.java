// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchdefinition.derived;

import com.yahoo.searchdefinition.parser.ParseException;
import org.junit.Test;

import java.io.IOException;

/**
 * Tests deriving of various field types
 *
 * @author  <a href="mailto:bratseth@yahoo-inc.com">Jon S Bratseth</a>
 */
public class PrefixExactAttributeTestCase extends AbstractExportingTestCase {
    @Test
    public void testTypes() throws IOException, ParseException {
        assertCorrectDeriving("prefixexactattribute");
    }
}
