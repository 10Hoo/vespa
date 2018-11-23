// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.search.dispatch;

import com.yahoo.fs4.QueryPacket;
import com.yahoo.prelude.fastsearch.CacheKey;
import com.yahoo.search.Query;
import com.yahoo.search.Result;
import com.yahoo.search.result.Coverage;
import com.yahoo.search.result.ErrorMessage;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

/**
 * A search invoker that will immediately produce an error that occurred during
 * invoker construction. Currently used for invalid searchpath values and node
 * failure
 *
 * @author ollivir
 */
public class SearchErrorInvoker extends SearchInvoker {
    private final ErrorMessage message;
    private Query query;
    private final Coverage coverage;
    private ResponseMonitor<SearchInvoker> monitor;

    public SearchErrorInvoker(ErrorMessage message, Coverage coverage) {
        super(Optional.empty());
        this.message = message;
        this.coverage = coverage;
    }

    public SearchErrorInvoker(ErrorMessage message) {
        this(message, null);
    }

    @Override
    protected void sendSearchRequest(Query query, QueryPacket queryPacket) throws IOException {
        this.query = query;
        if(monitor != null) {
            monitor.responseAvailable(this);
        }
    }

    @Override
    protected List<Result> getSearchResults(CacheKey cacheKey) throws IOException {
        Result res = new Result(query, message);
        if (coverage != null) {
            res.setCoverage(coverage);
        }
        return Arrays.asList(res);
    }

    @Override
    protected void release() {
        // nothing to do
    }

    @Override
    protected void setMonitor(ResponseMonitor<SearchInvoker> monitor) {
        this.monitor = monitor;
    }
}
