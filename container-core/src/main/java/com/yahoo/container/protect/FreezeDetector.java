// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.container.protect;

import java.util.Timer;

import com.yahoo.component.AbstractComponent;
import com.yahoo.concurrent.ThreadLocalDirectory;
import com.yahoo.container.core.DiagnosticsConfig;

/**
 * Runs and initializes a {@link Watchdog} instance.
 *
 * @author <a href="mailto:steinar@yahoo-inc.com">Steinar Knutsen</a>
 * @deprecated this is not in use and will be removed in the next major release
 */
@Deprecated
public class FreezeDetector extends AbstractComponent {

    private final Timer timeoutWatchdog;
    private final Watchdog watchdog;

    public FreezeDetector(DiagnosticsConfig diagnosticsConfig) {
        if (diagnosticsConfig.disabled()) {
            timeoutWatchdog = null;
            watchdog = null;
        } else {
            timeoutWatchdog = new Timer("TimeoutWatchdog", true);
            watchdog = new Watchdog(diagnosticsConfig.timeoutfraction(),
                    diagnosticsConfig.minimumqps(),
                    diagnosticsConfig.shutdown());
            timeoutWatchdog.schedule(watchdog, 10L * 1000L, 100L);
        }
    }

    public void register(ThreadLocalDirectory<TimeoutRate, Boolean> timeouts) {
        if (watchdog == null) {
            return;
        }
        watchdog.addTimeouts(timeouts);
    }

    public boolean isBreakdown() {
        if (watchdog == null) {
            return false;
        }
        return watchdog.isBreakdown();
    }

    public void unRegister(ThreadLocalDirectory<TimeoutRate, Boolean> timeouts) {
        if (watchdog == null) {
            return;
        }
        watchdog.removeTimeouts(timeouts);
    }

    @Override
    public void deconstruct() {
        super.deconstruct();
        if (timeoutWatchdog != null) {
            timeoutWatchdog.cancel();
        }
    }

}
