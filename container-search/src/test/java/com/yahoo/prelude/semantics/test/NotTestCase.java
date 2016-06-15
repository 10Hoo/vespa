// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.prelude.semantics.test;

/**
 * @author <a href="bratseth@yahoo-inc.com">Jon Bratseth</a>
 */
public class NotTestCase extends RuleBaseAbstractTestCase {

    public NotTestCase(String name) {
        super(name,"not.sr");
    }

    public void testLiteralEquals() {
        assertSemantics("RANK a foo:a","a");
        assertSemantics("a","a&ranking=category");
        assertSemantics("RANK a foo:a","a&ranking=somethingelse");
        assertSemantics("RANK a foo:a","a&otherparam=category");
    }

}
