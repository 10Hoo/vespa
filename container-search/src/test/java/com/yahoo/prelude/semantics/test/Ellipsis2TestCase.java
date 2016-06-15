// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.prelude.semantics.test;

/**
 * tersts the ellipsis rule base
 *
 * @author <a href="bratseth@yahoo-inc.com">Jon Bratseth</a>
 */
public class Ellipsis2TestCase extends RuleBaseAbstractTestCase {

    public Ellipsis2TestCase(String name) {
        super(name,"ellipsis2.sr");
    }

    public void testUnreferencedEllipsis() {
        assertSemantics("AND a b c someindex:\"a b c\"","a b c");
    }

}
