// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.orchestrator;

import com.yahoo.vespa.applicationmodel.ApplicationInstance;
import com.yahoo.vespa.applicationmodel.ApplicationInstanceId;
import com.yahoo.vespa.applicationmodel.ClusterId;
import com.yahoo.vespa.applicationmodel.ConfigId;
import com.yahoo.vespa.applicationmodel.HostName;
import com.yahoo.vespa.applicationmodel.ServiceCluster;
import com.yahoo.vespa.applicationmodel.ServiceInstance;
import com.yahoo.vespa.applicationmodel.ServiceType;
import com.yahoo.vespa.applicationmodel.TenantId;
import com.yahoo.vespa.service.monitor.ServiceMonitorStatus;
import org.junit.Test;

import java.util.HashSet;
import java.util.Optional;
import java.util.Set;

import static com.google.common.collect.Sets.newHashSet;
import static com.yahoo.vespa.orchestrator.TestUtil.makeServiceClusterSet;
import static com.yahoo.vespa.orchestrator.TestUtil.makeServiceInstanceSet;
import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;
import static org.fest.assertions.Assertions.assertThat;

/**
 * @author hakonhall
 */
public class VespaModelUtilTest {
    // Cluster Controller Service Cluster

    private static final ClusterId CONTENT_CLUSTER_ID = new ClusterId("content-cluster-0");

    public static final HostName controller0Host = new HostName("controller-0");

    private static final ServiceInstance<ServiceMonitorStatus> controller0 = new ServiceInstance<>(
            TestUtil.clusterControllerConfigId(0),
            controller0Host,
            ServiceMonitorStatus.UP);
    private static final ServiceInstance<ServiceMonitorStatus> controller1 = new ServiceInstance<>(
            TestUtil.clusterControllerConfigId(1),
            new HostName("controller-1"),
            ServiceMonitorStatus.UP);

    private static final ServiceCluster<ServiceMonitorStatus> controllerCluster =
            new ServiceCluster<>(
                    new ClusterId(CONTENT_CLUSTER_ID.s() + "-controller"),
                    VespaModelUtil.CLUSTER_CONTROLLER_SERVICE_TYPE,
                    makeServiceInstanceSet(controller1, controller0));

    // Distributor Service Cluster

    private static final ServiceInstance<ServiceMonitorStatus> distributor0 = new ServiceInstance<>(
            new ConfigId("distributor-config-id"),
            new HostName("distributor-0"),
            ServiceMonitorStatus.UP);


    private static final ServiceCluster<ServiceMonitorStatus> distributorCluster =
            new ServiceCluster<>(
                    CONTENT_CLUSTER_ID,
                    VespaModelUtil.DISTRIBUTOR_SERVICE_TYPE,
                    makeServiceInstanceSet(distributor0));

    // Storage Node Service Cluster

    public static final HostName storage0Host = new HostName("storage-0");
    private static final ServiceInstance<ServiceMonitorStatus> storage0 = new ServiceInstance<>(
            new ConfigId("storage-config-id"),
            storage0Host,
            ServiceMonitorStatus.UP);

    private static final ServiceCluster<ServiceMonitorStatus> storageCluster =
            new ServiceCluster<>(
                    CONTENT_CLUSTER_ID,
                    VespaModelUtil.STORAGENODE_SERVICE_TYPE,
                    makeServiceInstanceSet(storage0));

    // Secondary Distributor Service Cluster

    private static final ServiceInstance<ServiceMonitorStatus> secondaryDistributor0 = new ServiceInstance<>(
            new ConfigId("secondary-distributor-config-id"),
            new HostName("secondary-distributor-0"),
            ServiceMonitorStatus.UP);

    private static final ClusterId SECONDARY_CONTENT_CLUSTER_ID = new ClusterId("secondary-content-cluster-0");
    private static final ServiceCluster<ServiceMonitorStatus> secondaryDistributorCluster =
            new ServiceCluster<>(
                    SECONDARY_CONTENT_CLUSTER_ID,
                    VespaModelUtil.DISTRIBUTOR_SERVICE_TYPE,
                    makeServiceInstanceSet(secondaryDistributor0));

    // Secondary Storage Node Service Cluster

    public static final HostName secondaryStorage0Host = new HostName("secondary-storage-0");
    private static final ServiceInstance<ServiceMonitorStatus> secondaryStorage0 = new ServiceInstance<>(
            new ConfigId("secondary-storage-config-id"),
            secondaryStorage0Host,
            ServiceMonitorStatus.UP);

    private static final ServiceCluster<ServiceMonitorStatus> secondaryStorageCluster =
            new ServiceCluster<>(
                    SECONDARY_CONTENT_CLUSTER_ID,
                    VespaModelUtil.STORAGENODE_SERVICE_TYPE,
                    makeServiceInstanceSet(secondaryStorage0));

    // The Application Instance

    public static final ApplicationInstance<ServiceMonitorStatus> application =
            new ApplicationInstance<>(
                    new TenantId("tenant-0"),
                    new ApplicationInstanceId("application-0"),
                    makeServiceClusterSet(
                            controllerCluster,
                            distributorCluster,
                            storageCluster,
                            secondaryDistributorCluster,
                            secondaryStorageCluster));

    private ServiceCluster<?> createServiceCluster(ServiceType serviceType) {
        return new ServiceCluster<ServiceMonitorStatus>(
                new ClusterId("cluster-id"),
                serviceType,
                new HashSet<>());
    }

    @Test
    public void verifyControllerClusterIsRecognized() {
        ServiceCluster<?> cluster = createServiceCluster(VespaModelUtil.CLUSTER_CONTROLLER_SERVICE_TYPE);
        assertTrue(VespaModelUtil.isClusterController(cluster));
    }

    @Test
    public void verifyNonControllerClusterIsNotRecognized() {
        ServiceCluster<?> cluster = createServiceCluster(new ServiceType("foo"));
        assertFalse(VespaModelUtil.isClusterController(cluster));
    }

    @Test
    public void verifyStorageClusterIsRecognized() {
        ServiceCluster<?> cluster = createServiceCluster(VespaModelUtil.STORAGENODE_SERVICE_TYPE);
        assertTrue(VespaModelUtil.isStorage(cluster));
        cluster = createServiceCluster(VespaModelUtil.STORAGENODE_SERVICE_TYPE);
        assertTrue(VespaModelUtil.isStorage(cluster));
    }

    @Test
    public void verifyNonStorageClusterIsNotRecognized() {
        ServiceCluster<?> cluster = createServiceCluster(new ServiceType("foo"));
        assertFalse(VespaModelUtil.isStorage(cluster));
    }

    @Test
    public void verifyContentClusterIsRecognized() {
        ServiceCluster<?> cluster = createServiceCluster(VespaModelUtil.DISTRIBUTOR_SERVICE_TYPE);
        assertTrue(VespaModelUtil.isContent(cluster));
        cluster = createServiceCluster(VespaModelUtil.STORAGENODE_SERVICE_TYPE);
        assertTrue(VespaModelUtil.isContent(cluster));
        cluster = createServiceCluster(VespaModelUtil.SEARCHNODE_SERVICE_TYPE);
        assertTrue(VespaModelUtil.isContent(cluster));
    }

    @Test
    public void verifyNonContentClusterIsNotRecognized() {
        ServiceCluster<?> cluster = createServiceCluster(new ServiceType("foo"));
        assertFalse(VespaModelUtil.isContent(cluster));
    }

    @Test
    public void testGettingClusterControllerInstances() {
        Set<ServiceInstance<?>> controllers =
                new HashSet<>(VespaModelUtil.getClusterControllerInstances(application, CONTENT_CLUSTER_ID));
        Set<ServiceInstance<ServiceMonitorStatus>> expectedControllers = newHashSet(controller0, controller1);

        assertThat(controllers).isEqualTo(expectedControllers);
    }

    @Test
    public void testGetControllerHostName() {
        HostName host = VespaModelUtil.getControllerHostName(application, CONTENT_CLUSTER_ID);
        assertThat(host).isEqualTo(controller0Host);
    }

    @Test
    public void testGetContentClusterName() {
        ClusterId contentClusterName = VespaModelUtil.getContentClusterName(application, distributor0.hostName());
        assertThat(CONTENT_CLUSTER_ID).isEqualTo(contentClusterName);
    }

    @Test
    public void testGetContentClusterNameForSecondaryContentCluster() {
        ClusterId contentClusterName = VespaModelUtil.getContentClusterName(application, secondaryDistributor0.hostName());
        assertThat(SECONDARY_CONTENT_CLUSTER_ID).isEqualTo(contentClusterName);
    }

    @Test
    public void testGetStorageNodeAtHost() {
        Optional<ServiceInstance<ServiceMonitorStatus>> service =
                VespaModelUtil.getStorageNodeAtHost(application, storage0Host);
        assertTrue(service.isPresent());
        assertThat(service.get()).isEqualTo(storage0);
    }

    @Test
    public void testGetStorageNodeAtHostWithUnknownHost() {
        Optional<ServiceInstance<ServiceMonitorStatus>> service =
                VespaModelUtil.getStorageNodeAtHost(application, new HostName("storage-1"));
        assertFalse(service.isPresent());
    }

    @Test
    public void testGetClusterControllerIndex() {
        ConfigId configId = new ConfigId("admin/cluster-controllers/2");
        assertThat(VespaModelUtil.getClusterControllerIndex(configId)).isEqualTo(2);
    }

    @Test
    public void testGetStorageNodeIndex() {
        ConfigId configId = TestUtil.storageNodeConfigId(3);
        assertThat(VespaModelUtil.getStorageNodeIndex(configId)).isEqualTo(3);
    }
}
