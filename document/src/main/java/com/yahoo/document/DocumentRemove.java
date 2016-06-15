// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document;

/**
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 */
public class DocumentRemove extends DocumentOperation {

    private final DocumentId docId;

    public DocumentRemove(DocumentId docId) { this.docId = docId; }

    @Override
    public DocumentId getId() { return docId; }

    @Override
    public String toString() {
        return "DocumentRemove '" + docId + "'";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof DocumentRemove)) return false;
        DocumentRemove that = (DocumentRemove) o;
        if (!docId.equals(that.docId)) return false;
        return true;
    }

    @Override
    public int hashCode() {
        return docId.hashCode();
    }

}
