// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.jdisc;

/**
 * An implementation of {@link SharedResource} that does not do anything.
 * Useful base class for e.g. mocks of SharedResource sub-interfaces, where reference counting is not the focus.
 *
 * @author <a href="mailto:bakksjo@yahoo-inc.com">Oyvind Bakksjo</a>
 */
public class NoopSharedResource implements SharedResource {
    @Override
    public final ResourceReference refer() {
        return References.NOOP_REFERENCE;
    }

    @Override
    public final void release() {
    }
}
