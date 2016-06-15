// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.subscription;

import com.yahoo.config.ConfigInstance;
import com.yahoo.config.ConfigurationRuntimeException;
import com.yahoo.foo.SimpletypesConfig;
import com.yahoo.foo.AppConfig;
import com.yahoo.config.subscription.impl.ConfigSubscription;
import com.yahoo.vespa.config.ConfigKey;
import com.yahoo.vespa.config.TimingValues;

import org.junit.Ignore;
import org.junit.Test;

import java.util.Collections;
import java.util.List;

import static junit.framework.TestCase.fail;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.not;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertTrue;

/**
 * @author musum, lulf
 * @since 5.1
 */
public class ConfigSubscriptionTest {
    @Test
    public void testEquals() {
        ConfigSubscriber sub = new ConfigSubscriber();
        final String payload = "boolval true";
        ConfigSubscription<SimpletypesConfig> a = ConfigSubscription.get(new ConfigKey<>(SimpletypesConfig.class, "test"),
                sub, new RawSource(payload), new TimingValues());
        ConfigSubscription<SimpletypesConfig> b = ConfigSubscription.get(new ConfigKey<>(SimpletypesConfig.class, "test"),
                sub, new RawSource(payload), new TimingValues());
        ConfigSubscription<SimpletypesConfig> c = ConfigSubscription.get(new ConfigKey<>(SimpletypesConfig.class, "test2"),
                sub, new RawSource(payload), new TimingValues());
        assertThat(a, is(b));
        assertThat(a, is(a));
        assertThat(b, is(b));
        assertThat(c, is(c));
        assertThat(a, is(not(c)));
        assertThat(b, is(not(c)));

        ConfigSubscriber subscriber = new ConfigSubscriber();
        ConfigSet configSet = new ConfigSet();
        AppConfig.Builder a0builder = new AppConfig.Builder().message("A message, 0").times(88);
        configSet.addBuilder("app/0", a0builder);
        AppConfig.Builder a1builder = new AppConfig.Builder().message("A message, 1").times(89);
        configSet.addBuilder("app/1", a1builder);

        ConfigSubscription<AppConfig> c1 = ConfigSubscription.get(
                new ConfigKey<>(AppConfig.class, "app/0"),
                subscriber,
                configSet,
                new TimingValues());
        ConfigSubscription<AppConfig> c2 = ConfigSubscription.get(
                new ConfigKey<>(AppConfig.class, "app/1"),
                subscriber,
                configSet,
                new TimingValues());

        assertTrue(c1.equals(c1));
        assertFalse(c1.equals(c2));
    }

    @Test
    public void testSubscribeInterface() {
        ConfigSubscriber sub = new ConfigSubscriber();
        ConfigHandle<SimpletypesConfig> handle = sub.subscribe(SimpletypesConfig.class, "raw:boolval true", 10000);
        assertNotNull(handle);
        sub.nextConfig();
        assertThat(handle.getConfig().boolval(), is(true));
        //assertTrue(sub.getSource() instanceof RawSource);
    }

    // Test that subscription is closed and subscriptionHandles is empty if we get an exception (only the last is possible to
    // test right now).
    @Test
    @Ignore
    public void testSubscribeWithException() {
        TestConfigSubscriber sub = new TestConfigSubscriber();
        ConfigSourceSet configSourceSet = new ConfigSourceSet(Collections.singletonList("tcp/localhost:99999"));
        try {
            sub.subscribe(SimpletypesConfig.class, "configid", configSourceSet, new TimingValues().setSubscribeTimeout(100));
            fail();
        } catch (ConfigurationRuntimeException e) {
            assertThat(sub.getSubscriptionHandles().size(), is(0));
        }
    }

    private static class TestConfigSubscriber extends ConfigSubscriber {
        List<ConfigHandle<? extends ConfigInstance>> getSubscriptionHandles() {
            return subscriptionHandles;
        }
    }
}
