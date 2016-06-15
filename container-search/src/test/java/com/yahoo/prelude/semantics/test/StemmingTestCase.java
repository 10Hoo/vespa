// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.prelude.semantics.test;

/**
 * Tests a case reported by tularam
 *
 * @author <a href="mailto:bratseth@yahoo-inc.com">Jon Bratseth</a>
 */
public class StemmingTestCase extends RuleBaseAbstractTestCase {

    public StemmingTestCase(String name) {
        super(name,"stemming.sr");
    }

    public void testRewritingDueToStemmingInQuery() {
        assertSemantics("+i:vehicle -i:s","i:cars -i:s");
    }

    public void testRewritingDueToStemmingInRule() {
        assertSemantics("+i:animal -i:s","i:horse -i:s");
    }

    public void testRewritingDueToExactMatch() {
        assertSemantics("+(AND i:arts i:sciences) -i:s","i:as -i:s");
    }

    public void testNoRewritingBecauseShortWordsAreNotStemmed() {
        assertSemantics("+i:a -i:s","i:a -i:s");
    }

}
