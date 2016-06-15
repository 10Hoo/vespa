// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.plugin;

/**
 * This interface specifies an API for runtime-loadable server
 * plugins.  The interface is deliberately simple to allow it to be
 * used in different servers.  Typically, the initPlugin() method
 * calls application-specific registration methods to connect the
 * plugin to the hosting application.
 *
 * @author <a href="mailto:stig@yahoo-inc.com">Stig Bakken</a>
 */
public interface Plugin
{
    /**
     * @return a unique and simple name for the plugin
     */
    public String getPluginName();

    /**
     * Initialize the plugin.
     *
     * @param config Config object for this plugin
     */
    public void initPlugin(Config config);

    /**
     * Shut down the plugin.  Must clean up all resources allocated by
     * initPlugin() or any of the handler methods.
     */
    public void shutdownPlugin();

}
