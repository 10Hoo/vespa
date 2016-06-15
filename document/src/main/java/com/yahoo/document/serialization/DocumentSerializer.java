// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.serialization;

import com.yahoo.io.GrowableByteBuffer;

/**
 * Interface for serializing documents.
 *
 * A particular instance of this class is tied to a version of the document format.
 *
 * @author <a href="mailto:geirst@yahoo-inc.com">Geir Storli</a>
 */
public interface DocumentSerializer extends DocumentWriter, SpanNodeWriter, AnnotationWriter, SpanTreeWriter, DocumentUpdateWriter {

    /**
     * Returns the underlying buffer used for serialization.
     */
    public GrowableByteBuffer getBuf();
}

