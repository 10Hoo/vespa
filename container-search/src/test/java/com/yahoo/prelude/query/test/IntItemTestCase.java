// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.prelude.query.test;

import com.yahoo.prelude.query.AndItem;
import com.yahoo.prelude.query.IntItem;
import com.yahoo.search.Query;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * @author <a href="mailto:bratseth@yahoo-inc.com">Jon Bratseth</a>
 */
public class IntItemTestCase {

    @Test
    public void testEquals() {
        Query q1 = new Query("/?query=123%20456%20789");
        Query q2 = new Query("/?query=123%20456");

        AndItem andItem = (AndItem) q2.getModel().getQueryTree().getRoot();
        andItem.addItem(new IntItem(789l, ""));

        assertEquals(q1, q2);
    }

}
