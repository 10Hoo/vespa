// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.task.util.yum;

import com.yahoo.vespa.hosted.node.admin.component.TaskContext;
import com.yahoo.vespa.hosted.node.admin.task.util.process.ChildProcessFailureException;
import com.yahoo.vespa.hosted.node.admin.task.util.process.TestTerminal;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.Mockito.mock;

public class YumTest {
    private TaskContext taskContext = mock(TaskContext.class);
    private TestTerminal terminal = new TestTerminal();

    @Before
    public void tearDown() {
        terminal.verifyAllCommandsExecuted();
    }

    @Test
    public void testAlreadyInstalled() {
        terminal.expectCommand(
                "yum install --assumeyes --enablerepo=repo-name package-1 package-2 2>&1",
                0,
                "foobar\nNothing to do\n");

        Yum yum = new Yum(terminal);
        assertFalse(yum
                .install("package-1", "package-2")
                .enableRepo("repo-name")
                .converge(taskContext));
    }

    @Test
    public void testAlreadyUpgraded() {
        terminal.expectCommand(
                "yum upgrade --assumeyes package-1 package-2 2>&1",
                0,
                "foobar\nNo packages marked for update\n");

        assertFalse(new Yum(terminal)
                .upgrade("package-1", "package-2")
                .converge(taskContext));
    }

    @Test
    public void testAlreadyRemoved() {
        terminal.expectCommand(
                "yum remove --assumeyes package-1 package-2 2>&1",
                0,
                "foobar\nNo Packages marked for removal\n");

        assertFalse(new Yum(terminal)
                .remove("package-1", "package-2")
                .converge(taskContext));
    }

    @Test
    public void testInstall() {
        terminal.expectCommand(
                "yum install --assumeyes package-1 package-2 2>&1",
                0,
                "installing, installing");

        Yum yum = new Yum(terminal);
        assertTrue(yum
                .install("package-1", "package-2")
                .converge(taskContext));
    }

    @Test
    public void testInstallWithEnablerepo() {
        terminal.expectCommand(
                "yum install --assumeyes --enablerepo=repo-name package-1 package-2 2>&1",
                0,
                "installing, installing");

        Yum yum = new Yum(terminal);
        assertTrue(yum
                .install("package-1", "package-2")
                .enableRepo("repo-name")
                .converge(taskContext));
    }

    @Test(expected = ChildProcessFailureException.class)
    public void testFailedInstall() {
        terminal.expectCommand(
                "yum install --assumeyes --enablerepo=repo-name package-1 package-2 2>&1",
                1,
                "error");

        Yum yum = new Yum(terminal);
        yum.install("package-1", "package-2")
                .enableRepo("repo-name")
                .converge(taskContext);
        fail();
    }

    @Test
    public void testUnknownPackages() {
        terminal.expectCommand(
                "yum install --assumeyes package-1 package-2 package-3 2>&1",
                0,
                "Loaded plugins: fastestmirror, langpacks\n" +
                        "Loading mirror speeds from cached hostfile\n" +
                        "No package package-1 available.\n" +
                        "No package package-2 available.\n" +
                        "Nothing to do\n");

        Yum yum = new Yum(terminal);
        Yum.GenericYumCommand install = yum.install("package-1", "package-2", "package-3");

        try {
            install.converge(taskContext);
            fail();
        } catch (Exception e) {
            assertTrue(e.getCause() != null);
            assertEquals("Unknown package: package-1", e.getCause().getMessage());
        }
    }
}