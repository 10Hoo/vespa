// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.subscription.impl;

import com.yahoo.config.subscription.ConfigHandle;
import com.yahoo.vespa.config.RawConfig;

/**
 * A config handle which does not use the config class, but payload instead. To be used in proxy?
 *
 * @author vegardh
 */
@SuppressWarnings({"rawtypes", "unchecked"})
public class GenericConfigHandle extends ConfigHandle {

    private final GenericJRTConfigSubscription genSub;

    public GenericConfigHandle(GenericJRTConfigSubscription sub) {
        super(sub);
        genSub = sub;
    }

    public RawConfig getRawConfig() {
        return genSub.getRawConfig();
    }
}
