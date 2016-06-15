// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.documentapi.messagebus.protocol;

/**
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public class RemoveDocumentReply extends WriteDocumentReply {

    private boolean found = true;

    /**
     * Constructs a new reply with no content.
     */
    public RemoveDocumentReply() {
        super(DocumentProtocol.REPLY_REMOVEDOCUMENT);
    }

    /**
     * Returns whether or not the document was found and removed.
     *
     * @return True if document was found.
     */
    public boolean wasFound() {
        return found;
    }

    /**
     * Set whether or not the document was found and removed.
     *
     * @param found True if the document was found.
     */
    public void setWasFound(boolean found) {
        this.found = found;
    }
}
