// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespaxmlparser;

import com.yahoo.document.DocumentTypeManager;
import com.yahoo.vespa.http.client.config.FeedParams;
import com.yahoo.vespa.http.server.FeedReaderFactory;

import java.io.InputStream;

/**
 * For creating MockReader of innput stream.
 * @author dybdahl
 */
public class MockFeedReaderFactory extends FeedReaderFactory {

    @Override
    public FeedReader createReader(
            InputStream inputStream,
            DocumentTypeManager docTypeManager,
            FeedParams.DataFormat dataFormat) {
        try {
            return new MockReader(inputStream);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

}
