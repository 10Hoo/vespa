// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model.generic;

import com.yahoo.cloud.config.SentinelConfig;
import com.yahoo.config.codegen.CNode;
import com.yahoo.config.application.api.ApplicationPackage;
import com.yahoo.config.model.application.provider.FilesApplicationPackage;
import com.yahoo.vespa.config.ConfigDefinitionKey;
import com.yahoo.vespa.config.ConfigPayload;
import com.yahoo.vespa.config.ConfigPayloadBuilder;
import com.yahoo.vespa.config.GenericConfig;
import com.yahoo.vespa.defaults.Defaults;
import com.yahoo.vespa.model.VespaModel;
import com.yahoo.vespa.model.generic.service.Service;
import com.yahoo.vespa.model.generic.service.ServiceCluster;
import org.junit.BeforeClass;
import org.junit.Test;
import org.xml.sax.SAXException;

import java.io.File;
import java.io.IOException;
import java.util.Iterator;

import static org.junit.Assert.*;

/**
 * Tests that generic services result in correct sentinel config settings
 *
 * @author vegardh
 */
public class GenericServicesTest {

    private static VespaModel model;

    @BeforeClass
    public static void getModel() throws IOException, SAXException {
        String appDir = "src/test/cfg/application/app_genericservices";
        ApplicationPackage app = FilesApplicationPackage.fromFile(new File(appDir));
        model = new VespaModel(app);
    }

    @Test
    public void testServicesSentinelConfig() throws IOException, SAXException {
        String sentinelConfigId1="hosts/bogusname1/sentinel";
        String sentinelConfigId2="hosts/bogusname2/sentinel";
        String sentinelConfigId3="hosts/bogusname3/sentinel";
        String sentinelConfigId4="hosts/bogusname4/sentinel";
        SentinelConfig sentinel1 = model.getConfig(SentinelConfig.class, sentinelConfigId1);
        SentinelConfig sentinel2 = model.getConfig(SentinelConfig.class, sentinelConfigId2);
        SentinelConfig sentinel3 = model.getConfig(SentinelConfig.class, sentinelConfigId3);
        SentinelConfig sentinel4 = model.getConfig(SentinelConfig.class, sentinelConfigId4);

        assertServiceExists(sentinel1, "myservice", "mycmd1.sh", "myservice/0", true, true);
        assertServiceExists(sentinel2, "myservice", "mycmd1.sh", "myservice/1", true, true);
        assertServiceExists(sentinel3, "myservice", "mycmd1.sh", "myservice/2", true, true);
        assertServiceExists(sentinel3, "myservice2", "mycmd1.sh", "myservice/3", true, true);
        assertServiceExists(sentinel3, "myotherservice", "/home/vespa/bin/mycmd2.sh --ytest $FOO_BAR", "myotherservice/0", true, true);
        assertServiceExists(sentinel4, "myotherservice", "/home/vespa/bin/mycmd2.sh --ytest $FOO_BAR", "myotherservice/1", true, true);
    }

    private void assertServiceExists(SentinelConfig sentinel, String serviceName, String cmd, String configId, boolean autostart, boolean autorestart) {
        boolean matches = false;
        Iterator<SentinelConfig.Service> it = sentinel.service().iterator();
        while (!matches && it.hasNext()) {
            SentinelConfig.Service service = it.next();
            matches = service.autorestart() == autorestart &&
                service.autostart() == autostart &&
                service.name().equals(serviceName) &&
                service.id().equals(configId) &&
                service.command().equals(cmd);
        }
        assertTrue(matches);
    }

    @Test
    public void testServicesModel() throws IOException, SAXException {
        // Testing that this model can be constructed only for now
    }

}
