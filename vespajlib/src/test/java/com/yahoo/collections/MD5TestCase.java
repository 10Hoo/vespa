// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.collections;

/**
 * @author <a href="mailto:einarmr@yahoo-inc.com">Einar M R Rosenvinge</a>
 */
public class MD5TestCase extends junit.framework.TestCase {
    public void testMD5() {
        MD5 md5 = new MD5();
        int a = md5.hash("foobar");
        int b = md5.hash("foobar");

        assertEquals(a, b);

        int c = md5.hash("foo");

        assertTrue(a != c);
        assertTrue(b != c);

        //rudimentary check; see that all four bytes contain something:

        assertTrue((a & 0xFF000000) != 0);
        assertTrue((a & 0x00FF0000) != 0);
        assertTrue((a & 0x0000FF00) != 0);
        assertTrue((a & 0x000000FF) != 0);


        assertTrue((c & 0xFF000000) != 0);
        assertTrue((c & 0x00FF0000) != 0);
        assertTrue((c & 0x0000FF00) != 0);
        assertTrue((c & 0x000000FF) != 0);
    }
}
