// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.maintenance;

import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.yahoo.config.provision.Environment;
import com.yahoo.config.provision.SystemName;
import com.yahoo.vespa.hosted.controller.Application;
import com.yahoo.vespa.hosted.controller.Controller;
import com.yahoo.vespa.hosted.controller.ControllerTester;
import com.yahoo.vespa.hosted.controller.MetricsMock;
import com.yahoo.vespa.hosted.controller.MetricsMock.MapContext;
import com.yahoo.vespa.hosted.controller.api.integration.chef.AttributeMapping;
import com.yahoo.vespa.hosted.controller.api.integration.chef.Chef;
import com.yahoo.vespa.hosted.controller.api.integration.chef.rest.PartialNodeResult;
import com.yahoo.vespa.hosted.controller.application.ApplicationPackage;
import com.yahoo.vespa.hosted.controller.deployment.ApplicationPackageBuilder;
import com.yahoo.vespa.hosted.controller.deployment.DeploymentTester;
import com.yahoo.vespa.hosted.controller.persistence.MockCuratorDb;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.Clock;
import java.time.Instant;
import java.time.ZoneId;
import java.util.Map;

import static com.yahoo.vespa.hosted.controller.application.DeploymentJobs.JobType.component;
import static com.yahoo.vespa.hosted.controller.application.DeploymentJobs.JobType.systemTest;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.mockito.Matchers.anyListOf;
import static org.mockito.Matchers.anyString;
import static org.mockito.Mockito.when;

/**
 * @author mortent
 */
public class MetricsReporterTest {

    private static final Path testData = Paths.get("src/test/resources/");
    private MetricsMock metrics;

    @Before
    public void before() {
        metrics = new MetricsMock();
    }

    @Test
    public void test_chef_metrics() {
        ControllerTester tester = new ControllerTester();
        MetricsReporter metricsReporter = createReporter(tester.controller(), metrics, SystemName.cd);
        metricsReporter.maintain();
        assertEquals(2, metrics.getMetrics().size());

        Map<MapContext, Map<String, Number>> hostMetrics = getMetricsByHost("fake-node.test");
        assertEquals(1, hostMetrics.size());
        Map.Entry<MapContext, Map<String, Number>> metricEntry = hostMetrics.entrySet().iterator().next();
        MapContext metricContext = metricEntry.getKey();
        assertDimension(metricContext, "tenantName", "ciintegrationtests");
        assertDimension(metricContext, "app", "restart.default");
        assertDimension(metricContext, "zone", "prod.cd-us-east-1");
        assertEquals(727, metricEntry.getValue().get(MetricsReporter.convergeMetric).longValue());
    }

    @Test
    public void test_deployment_fail_ratio() {
        DeploymentTester tester = new DeploymentTester();
        ApplicationPackage applicationPackage = new ApplicationPackageBuilder()
                .environment(Environment.prod)
                .region("us-west-1")
                .build();
        MetricsReporter metricsReporter = createReporter(tester.controller(), metrics, SystemName.cd);

        metricsReporter.maintain();
        assertEquals(0.0, metrics.getMetric(MetricsReporter.deploymentFailMetric));

        // Deploy all apps successfully
        Application app1 = tester.createApplication("app1", "tenant1", 1, 11L);
        Application app2 = tester.createApplication("app2", "tenant1", 2, 22L);
        Application app3 = tester.createApplication("app3", "tenant1", 3, 33L);
        Application app4 = tester.createApplication("app4", "tenant1", 4, 44L);
        tester.deployCompletely(app1, applicationPackage);
        tester.deployCompletely(app2, applicationPackage);
        tester.deployCompletely(app3, applicationPackage);
        tester.deployCompletely(app4, applicationPackage);

        metricsReporter.maintain();
        assertEquals(0.0, metrics.getMetric(MetricsReporter.deploymentFailMetric));

        // 1 app fails system-test
        tester.jobCompletion(component).application(app4).submit();
        tester.deployAndNotify(app4, applicationPackage, false, systemTest);

        metricsReporter.maintain();
        assertEquals(25.0, metrics.getMetric(MetricsReporter.deploymentFailMetric));
    }

    @Test
    public void it_omits_zone_when_unknown() {
        ControllerTester tester = new ControllerTester();
        String hostname = "fake-node2.test";
        MapContext metricContext = getMetricContextByHost(tester.controller(), hostname);
        assertNull(metricContext.getDimensions().get("zone"));
    }

    private void assertDimension(MapContext metricContext, String dimensionName, String expectedValue) {
        assertEquals(expectedValue, metricContext.getDimensions().get(dimensionName));
    }

    private MetricsReporter createReporter(Controller controller, MetricsMock metricsMock, SystemName system) {
        Chef client = Mockito.mock(Chef.class);
        PartialNodeResult result;
        try {
            result = new ObjectMapper()
                    .configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false)
                    .readValue(testData.resolve("chef_output.json").toFile(), PartialNodeResult.class);
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
        when(client.partialSearchNodes(anyString(), anyListOf(AttributeMapping.class))).thenReturn(result);

        Clock clock = Clock.fixed(Instant.ofEpochSecond(1475497913), ZoneId.systemDefault());

        return new MetricsReporter(controller, metricsMock, client, clock, new JobControl(new MockCuratorDb()), system);
    }

    private Map<MapContext, Map<String, Number>> getMetricsByHost(String hostname) {
        return metrics.getMetrics((dimension, value) -> dimension.equals("host") && value.equals(hostname));
    }
    
    private MapContext getMetricContextByHost(Controller controller, String hostname) {
        MetricsReporter metricsReporter = createReporter(controller, metrics, SystemName.main);
        metricsReporter.maintain();

        assertFalse(metrics.getMetrics().isEmpty());

        Map<MapContext, Map<String, Number>> metrics = getMetricsByHost(hostname);
        assertEquals(1, metrics.size());
        Map.Entry<MapContext, Map<String, Number>> metricEntry = metrics.entrySet().iterator().next();
        return metricEntry.getKey();
    }

}

