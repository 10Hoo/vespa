// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.prelude.semantics.test;

import com.yahoo.prelude.semantics.RuleBaseException;
import com.yahoo.prelude.semantics.RuleImporter;
import com.yahoo.prelude.semantics.parser.ParseException;

/**
 * @author <a href="mailto:bratseth@yahoo-inc.com">Jon S Bratseth</a>
 */
public class DuplicateRuleTestCase extends junit.framework.TestCase {

    private final String root="src/test/java/com/yahoo/prelude/semantics/test/rulebases/";

    public DuplicateRuleTestCase(String name) {
        super(name);
    }

    public void testDuplicateRuleBaseLoading() throws java.io.IOException, ParseException  {
        if (System.currentTimeMillis() > 0) return; // TODO: Include this test...

        try {
            new RuleImporter().importFile(root + "rules.sr");
            fail("Did not detect duplicate condition names");
        }
        catch (RuleBaseException e) {
            assertEquals("Duplicate condition 'something' in 'duplicaterules.sr'",e.getMessage());
        }
    }

}
