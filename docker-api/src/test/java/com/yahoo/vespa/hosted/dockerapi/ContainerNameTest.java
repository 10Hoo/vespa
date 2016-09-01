package com.yahoo.vespa.hosted.dockerapi;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * @author valerijf
 */
public class ContainerNameTest {
    @Test
    public void testAlphanumericalContainerName() {
        String name = "container123";
        ContainerName containerName = new ContainerName(name);
        assertEquals(containerName.asString(), name);
    }

    @Test
    public void testAlphanumericalWithDashContainerName() {
        String name = "container-123";
        ContainerName containerName = new ContainerName(name);
        assertEquals(containerName.asString(), name);
    }

    @Test(expected=IllegalArgumentException.class)
    public void testAlphanumericalWithSlashContainerName() {
        new ContainerName("container/123");
    }

    @Test(expected=IllegalArgumentException.class)
    public void testEmptyContainerName() {
        new ContainerName("");
    }

    @Test(expected=NullPointerException.class)
    public void testNullContainerName() {
        new ContainerName(null);
    }
}
