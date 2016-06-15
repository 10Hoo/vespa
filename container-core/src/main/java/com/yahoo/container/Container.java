// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.container;

import com.yahoo.component.AbstractComponent;
import com.yahoo.component.provider.ComponentRegistry;
import com.yahoo.config.FileReference;
import com.yahoo.container.core.config.BundleLoader;
import com.yahoo.container.osgi.AbstractRpcAdaptor;
import com.yahoo.container.osgi.ContainerRpcAdaptor;
import com.yahoo.filedistribution.fileacquirer.FileAcquirer;
import com.yahoo.filedistribution.fileacquirer.FileAcquirerFactory;
import com.yahoo.jdisc.handler.RequestHandler;
import com.yahoo.jdisc.service.ClientProvider;
import com.yahoo.jdisc.service.ServerProvider;
import com.yahoo.osgi.Osgi;
import com.yahoo.vespa.config.ConfigTransformer;
import com.yahoo.vespa.config.ConfigTransformer.PathAcquirer;

import java.nio.file.Path;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

/**
 * The container instance. This is a Vespa internal object, external code should
 * only depend on this if there are no other options, and must be prepared to
 * see it change at no warning.
 *
 * @author bratseth
 */
public class Container {

    private volatile boolean usingCustomFileAcquirer = false;

    private volatile ComponentRegistry<RequestHandler> requestHandlerRegistry;
    private volatile ComponentRegistry<ClientProvider> clientProviderRegistry;
    private volatile ComponentRegistry<ServerProvider> serverProviderRegistry;
    private volatile ComponentRegistry<AbstractComponent> componentRegistry;
    private volatile FileAcquirer fileAcquirer;
    private Osgi osgi;

    private final ContainerRpcAdaptor rpcAdaptor = new ContainerRpcAdaptor(osgi);

    private volatile BundleLoader bundleLoader;

    private static Logger logger = Logger.getLogger(Container.class.getName());

    //TODO: Make this final again.
    private static Container instance = new Container();

    public static Container get() { return instance; }

    public void setOsgi(Osgi osgi) {
        this.osgi = osgi;
        bundleLoader = new BundleLoader(osgi);
    }

    public void shutdown() {
        com.yahoo.container.Server.get().shutdown();
        if (fileAcquirer != null)
            fileAcquirer.shutdown();

        rpcAdaptor.shutdown();
    }

    /** Returns the rpc adaptor owned by this */
    public ContainerRpcAdaptor getRpcAdaptor() {
        return rpcAdaptor;
    }

    //Used to acquire files originating from the application package.
    public FileAcquirer getFileAcquirer() {
        return fileAcquirer;
    }

    public BundleLoader getBundleLoader() {
        if (bundleLoader == null)
            bundleLoader = new BundleLoader(null);
        return bundleLoader;
    }

    /** Hack. For internal use only, will be removed later
     *
     * Used by LocalApplication to be able to repeatedly set up containers.
     **/
    public static void resetInstance() {
        instance = new Container();
        com.yahoo.container.Server.resetInstance();
    }

    /**
     * Add an application specific RPC adaptor.
     *
     * @param adaptor the RPC adaptor to add to the Container
     */
    public void addOptionalRpcAdaptor(AbstractRpcAdaptor adaptor) {
        rpcAdaptor.bindRpcAdaptor(adaptor);
    }

    public ComponentRegistry<RequestHandler> getRequestHandlerRegistry() {
        return requestHandlerRegistry;
    }

    public void setRequestHandlerRegistry(ComponentRegistry<RequestHandler> requestHandlerRegistry) {
        this.requestHandlerRegistry = requestHandlerRegistry;
    }

    public ComponentRegistry<ClientProvider> getClientProviderRegistry() {
        return clientProviderRegistry;
    }

    public void setClientProviderRegistry(ComponentRegistry<ClientProvider> clientProviderRegistry) {
        this.clientProviderRegistry = clientProviderRegistry;
    }

    public ComponentRegistry<ServerProvider> getServerProviderRegistry() {
        return serverProviderRegistry;
    }

    public void setServerProviderRegistry(ComponentRegistry<ServerProvider> serverProviderRegistry) {
        this.serverProviderRegistry = serverProviderRegistry;
    }

    public ComponentRegistry<AbstractComponent> getComponentRegistry() {
        return componentRegistry;
    }

    public void setComponentRegistry(ComponentRegistry<AbstractComponent> registry) {
        registry.freeze();
        this.componentRegistry = registry;
    }

    //Only intended for use by the Server instance.
    public void setupFileAcquirer(QrConfig.Filedistributor filedistributorConfig) {
        if (usingCustomFileAcquirer)
            return;

        if (filedistributorConfig.configid().isEmpty()) {
            if (fileAcquirer != null)
                logger.warning("Disabling file distribution");
            fileAcquirer = null;
        } else {
            fileAcquirer = FileAcquirerFactory.create(filedistributorConfig.configid());
        }

        setPathAcquirer(fileAcquirer);
    }

    /** Just a helper method to return a useful host to bind to. */
    public static String bindHostName(String host) {
        if ("".equals(host)) {
            return "0.0.0.0";
        } else {
            return host;
        }
    }

    /**
     * Only for internal use.
     */
    public void setCustomFileAcquirer(final FileAcquirer fileAcquirer) {
        if (this.fileAcquirer != null) {
            throw new RuntimeException("Can't change file acquirer. Is " +
                                       this.fileAcquirer + " attempted to set to " + fileAcquirer);
        }
        usingCustomFileAcquirer = true;
        this.fileAcquirer = fileAcquirer;
        setPathAcquirer(fileAcquirer);
    }

    private void setPathAcquirer(final FileAcquirer fileAcquirer) {
        ConfigTransformer.setPathAcquirer(new PathAcquirer() {
            @Override
            public Path getPath(FileReference fileReference) {
                try {
                    return fileAcquirer.waitFor(fileReference, 15, TimeUnit.MINUTES).toPath();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    throw new RuntimeException(e);
                }
            }
        });
    }
}
