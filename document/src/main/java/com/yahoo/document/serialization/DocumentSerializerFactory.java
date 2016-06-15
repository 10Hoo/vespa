// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.serialization;

import com.yahoo.io.GrowableByteBuffer;

/**
 * Factory for creating document serializers tied to a document format.
 *
 * @author <a href="mailto:geirst@yahoo-inc.com">Geir Storli</a>
 */
public class DocumentSerializerFactory {

    /**
     * Creates a serializer for the current head document format.
     * This format is an extension of the 4.2 format.
     */
    public static DocumentSerializer createHead(GrowableByteBuffer buf) {
        return new VespaDocumentSerializerHead(buf);
    }

    /**
     * Creates a serializer for the document format that was created on Vespa 4.2.
     */
    public static DocumentSerializer create42(GrowableByteBuffer buf) {
        return new VespaDocumentSerializer42(buf);
    }

    /**
     * Creates a serializer for the document format that was created on Vespa 4.2.
     */
    public static DocumentSerializer create42(GrowableByteBuffer buf, boolean headerOnly) {
        return new VespaDocumentSerializer42(buf, headerOnly);
    }

    /**
     * Creates a serializer for the document format that was created on Vespa 4.2.
     */
    public static DocumentSerializer create42() {
        return new VespaDocumentSerializer42();
    }

}
