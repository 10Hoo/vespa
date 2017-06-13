// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.jrt;

public class QueueTest extends junit.framework.TestCase {

    public QueueTest(String name) {
        super(name);
    }

    public void testEmpty() {
        Queue queue = new Queue();

        assertTrue(queue.isEmpty());
        assertTrue(queue.size() == 0);
        assertTrue(queue.dequeue() == null);
        queue.enqueue(new Object());
        assertFalse(queue.isEmpty());
        assertFalse(queue.size() == 0);
        assertFalse(queue.dequeue() == null);
    }

    public void testEnqueueDequeue() {
        Queue queue = new Queue();
        Integer int1 = new Integer(1);
        Integer int2 = new Integer(2);
        Integer int3 = new Integer(3);
        Integer int4 = new Integer(4);
        Integer int5 = new Integer(5);

        assertEquals(queue.size(), 0);
        queue.enqueue(int1);
        assertEquals(queue.size(), 1);
        assertTrue(queue.dequeue() == int1);
        assertEquals(queue.size(), 0);

        queue.enqueue(int1);
        assertEquals(queue.size(), 1);
        queue.enqueue(int2);
        assertEquals(queue.size(), 2);
        queue.enqueue(int3);
        assertEquals(queue.size(), 3);
        assertTrue(queue.dequeue() == int1);
        assertEquals(queue.size(), 2);
        assertTrue(queue.dequeue() == int2);
        assertEquals(queue.size(), 1);
        assertTrue(queue.dequeue() == int3);
        assertEquals(queue.size(), 0);

        queue.enqueue(int1);
        assertEquals(queue.size(), 1);
        queue.enqueue(int2);
        assertEquals(queue.size(), 2);
        queue.enqueue(int3);
        assertEquals(queue.size(), 3);
        assertTrue(queue.dequeue() == int1);
        assertEquals(queue.size(), 2);
        assertTrue(queue.dequeue() == int2);
        assertEquals(queue.size(), 1);
        queue.enqueue(int4);
        assertEquals(queue.size(), 2);
        queue.enqueue(int5);
        assertEquals(queue.size(), 3);

        assertTrue(queue.dequeue() == int3);
        assertEquals(queue.size(), 2);
        assertTrue(queue.dequeue() == int4);
        assertEquals(queue.size(), 1);
        assertTrue(queue.dequeue() == int5);
        assertEquals(queue.size(), 0);
    }

    public void testFlush() {
        Queue src = new Queue();
        Queue dst = new Queue();
        Integer int1 = new Integer(1);
        Integer int2 = new Integer(2);
        Integer int3 = new Integer(3);

        assertTrue(src.flush(dst) == 0);
        assertEquals(src.size(), 0);
        assertEquals(dst.size(), 0);

        src.enqueue(int1);
        src.enqueue(int2);
        src.enqueue(int3);

        assertEquals(src.size(), 3);
        assertEquals(dst.size(), 0);
        assertTrue(src.flush(dst) == 3);
        assertEquals(src.size(), 0);
        assertEquals(dst.size(), 3);

        assertTrue(dst.dequeue() == int1);
        assertTrue(dst.dequeue() == int2);
        assertTrue(dst.dequeue() == int3);
    }
}
