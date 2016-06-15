// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.logserver;

import com.yahoo.log.LogSetup;
import com.yahoo.logserver.Server;
import com.yahoo.logserver.handlers.LogHandler;
import com.yahoo.logserver.handlers.logmetrics.LogMetricsPlugin;
import com.yahoo.logserver.test.LogDispatcherTestCase;

import java.io.IOException;

import org.junit.*;
import static org.junit.Assert.*;

/**
 * Unit tests for the Server class.
 *
 * @author  <a href="mailto:borud@yahoo-inc.com">Bjorn Borud</a>
 */
public class ServerTestCase {

    @Test
	public void testStartupAndRegHandlers() throws IOException, InterruptedException {
        Server.help();
        Server server = Server.getInstance();
        server.initialize(18322);
        LogSetup.clearHandlers();
        Thread serverThread = new Thread(server);
        serverThread.start();
        assertTrue(serverThread.isAlive());
        LogHandler handler = new LogDispatcherTestCase.MockHandler();
        server.registerLogHandler(handler , "foo");
        assertEquals(Server.threadNameForHandler().get(handler), "foo");
        server.unregisterLogHandler(handler);
        assertEquals(Server.threadNameForHandler().get(handler), null);
		serverThread.interrupt();
		try {
			serverThread.join();
			assertTrue(true);
		}
		catch (InterruptedException e) {
			fail();
		}
	}
    
    @Test
    public void testPluginLoaderClassLoading() {
        AbstractPluginLoader loader = new BuiltinPluginLoader();
        System.setProperty("logserver.logmetrics.enable", "false");
        loader.loadFromClass(LogMetricsPlugin.class);
        System.setProperty("logserver.logmetrics.enable", "true");
        loader.loadFromClass(LogMetricsPlugin.class); // Hm, no way to verify it was loaded
    }
    
}
