// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.service.duper;

import com.yahoo.cloud.config.ConfigserverConfig;
import com.yahoo.config.model.api.ApplicationInfo;
import com.yahoo.config.model.api.SuperModel;
import com.yahoo.config.model.api.SuperModelListener;
import com.yahoo.config.model.api.SuperModelProvider;
import com.yahoo.config.provision.ApplicationId;
import com.yahoo.config.provision.HostName;
import com.yahoo.vespa.service.monitor.ConfigserverUtil;
import org.junit.Test;
import org.mockito.ArgumentCaptor;

import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static org.mockito.Matchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * @author hakonhall
 */
public class DuperModelManagerTest {
    private final ConfigserverConfig configserverConfig = ConfigserverUtil.createExampleConfigserverConfig();
    private final ApplicationInfo configServerApplicationFromConfig = new ConfigServerApplication().makeApplicationInfoFromConfig(configserverConfig);
    private final SuperModelProvider superModelProvider = mock(SuperModelProvider.class);
    private final SuperModel superModel = mock(SuperModel.class);
    private final DuperModel duperModel = mock(DuperModel.class);

    private DuperModelManager manager;
    private SuperModelListener superModelListener;

    private void makeManager() {
        makeManager(true);
    }

    private void makeManager(boolean containsInfra) {
        makeManager(containsInfra, true);
    }

    private void makeManager(boolean containsInfra, boolean useConfigServerConfig) {
        manager = new DuperModelManager(
                containsInfra,
                useConfigServerConfig,
                true,
                configServerApplicationFromConfig,
                superModelProvider,
                duperModel);

        when(superModelProvider.getSuperModel()).thenReturn(superModel);
        verify(duperModel, times(useConfigServerConfig ? 1 : 0)).add(any());

        ArgumentCaptor<SuperModelListener> superModelListenerCaptor = ArgumentCaptor.forClass(SuperModelListener.class);
        verify(superModelProvider, times(1)).registerListener(superModelListenerCaptor.capture());
        superModelListener = superModelListenerCaptor.getValue();
    }

    @Test
    public void testSuperModelAffectsDuperModel() {
        makeManager();

        superModelListener.applicationActivated(superModel, mock(ApplicationInfo.class));
        verify(duperModel, times(2)).add(any());

        verify(duperModel, times(0)).remove(any());
        superModelListener.applicationRemoved(superModel, ApplicationId.from("tenant", "app", "default"));
        verify(duperModel, times(1)).remove(any());
    }

    @Test
    public void testInfraApplicationsAffectsDuperModel() {
        makeManager();

        ApplicationId id = manager.getProxyHostApplication().getApplicationId();
        List<HostName> proxyHostHosts = Stream.of("proxyhost1", "proxyhost2").map(HostName::from).collect(Collectors.toList());
        manager.infraApplicationActivated(id, proxyHostHosts);
        verify(duperModel, times(2)).add(any());
        when(duperModel.contains(id)).thenReturn(true);

        verify(duperModel, times(0)).remove(any());
        manager.infraApplicationRemoved(id);
        verify(duperModel, times(1)).remove(any());
    }

    @Test
    public void testDisableInfraApplications() {
        makeManager(false);

        List<HostName> proxyHostHosts = Stream.of("proxyhost1", "proxyhost2").map(HostName::from).collect(Collectors.toList());
        manager.infraApplicationActivated(manager.getProxyHostApplication().getApplicationId(), proxyHostHosts);
        verify(duperModel, times(1)).add(any());

        verify(duperModel, times(0)).remove(any());
        manager.infraApplicationRemoved(manager.getProxyHostApplication().getApplicationId());
        verify(duperModel, times(0)).remove(any());
    }

    @Test
    public void testConfigServerInfraApplications() {
        makeManager();
        testConfigServerLikeInfraApplication(manager.getConfigServerApplication().getApplicationId());
    }

    @Test
    public void testControllerInfraApplications() {
        makeManager();
        testConfigServerLikeInfraApplication(manager.getControllerApplication().getApplicationId());
    }

    private void testConfigServerLikeInfraApplication(ApplicationId configServerLikeId) {
        List<HostName> hostnames = Stream.of("node1", "node2").map(HostName::from).collect(Collectors.toList());
        manager.infraApplicationActivated(configServerLikeId, hostnames);
        verify(duperModel, times(1)).add(any());

        verify(duperModel, times(0)).remove(any());
        manager.infraApplicationRemoved(configServerLikeId);
        verify(duperModel, times(0)).remove(any());
    }

    @Test
    public void testEnabledConfigServerInfraApplications() {
        makeManager(true, false);
        testEnabledConfigServerLikeInfraApplication(
                manager.getConfigServerApplication().getApplicationId(),
                manager.getControllerApplication().getApplicationId());
    }

    @Test
    public void testEnabledControllerInfraApplications() {
        makeManager(true, false);
        testEnabledConfigServerLikeInfraApplication(
                manager.getControllerApplication().getApplicationId(),
                manager.getConfigServerApplication().getApplicationId());
    }

    private void testEnabledConfigServerLikeInfraApplication(ApplicationId firstId, ApplicationId secondId) {
        List<HostName> hostnames1 = Stream.of("node11", "node12").map(HostName::from).collect(Collectors.toList());
        manager.infraApplicationActivated(firstId, hostnames1);
        verify(duperModel, times(1)).add(any());
        when(duperModel.contains(firstId)).thenReturn(true);

        // Adding the second config server like application will be ignored
        List<HostName> hostnames2 = Stream.of("node21", "node22").map(HostName::from).collect(Collectors.toList());
        manager.infraApplicationActivated(secondId, hostnames2);
        verify(duperModel, times(1)).add(any());

        verify(duperModel, times(0)).remove(any());
        manager.infraApplicationRemoved(firstId);
        verify(duperModel, times(1)).remove(any());
        when(duperModel.contains(firstId)).thenReturn(false);

        // Removing the second config server like application cannot be removed since it wasn't added
        verify(duperModel, times(1)).remove(any());
        manager.infraApplicationRemoved(secondId);
        verify(duperModel, times(1)).remove(any());
    }

    @Test
    public void testSingleTenant() {
        manager = new DuperModelManager(
                true,
                true,
                false,
                configServerApplicationFromConfig,
                superModelProvider,
                duperModel);

        when(superModelProvider.getSuperModel()).thenReturn(superModel);
        verify(duperModel, times(0)).add(any());

        List<HostName> hostnames = Stream.of("node1", "node2").map(HostName::from).collect(Collectors.toList());
        manager.infraApplicationActivated(manager.getConfigServerApplication().getApplicationId(), hostnames);
        verify(duperModel, times(0)).add(any());

        verify(duperModel, times(0)).remove(any());
        manager.infraApplicationRemoved(manager.getConfigServerApplication().getApplicationId());
        verify(duperModel, times(0)).remove(any());
    }
}