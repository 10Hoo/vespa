// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vdslib;

import com.yahoo.document.Document;
import com.yahoo.document.DocumentPut;
import com.yahoo.document.DocumentType;
import com.yahoo.document.DocumentTypeManager;
import com.yahoo.document.DocumentTypeManagerConfigurer;

/**
 * @author banino
 */
public class EntryTestCase extends junit.framework.TestCase{

    public void testEquals() {
        DocumentTypeManager manager = new DocumentTypeManager();
        DocumentTypeManagerConfigurer.configure(manager, "file:src/test/files/documentmanager.cfg");

        DocumentType bmType = manager.getDocumentType("benchmark");
        DocumentPut put1 = new DocumentPut(bmType, "userdoc:foo:99999999:1");
        DocumentPut put2 = new DocumentPut(bmType, "userdoc:foo:99999999:2");

        Entry entry1 = Entry.create(put1);
        Entry entry2 = Entry.create(put1);
        assert(entry1.equals(entry2));

        Entry entry3 = Entry.create(put2);
        assert(!entry1.equals(entry3));

    }

    public void testHashCode() {
        DocumentTypeManager manager = new DocumentTypeManager();
        DocumentTypeManagerConfigurer.configure(manager, "file:src/test/files/documentmanager.cfg");

        DocumentType bmType = manager.getDocumentType("benchmark");
        DocumentPut put1 = new DocumentPut(bmType, "userdoc:foo:99999999:1");
        DocumentPut put2 = new DocumentPut(bmType, "userdoc:foo:99999999:2");

        Entry entry1 = Entry.create(put1);
        Entry entry2 = Entry.create(put1);
        assert(entry1.hashCode() == entry2.hashCode());

        Entry entry3 = Entry.create(put2);
        assert(entry1.hashCode() != entry3.hashCode());

    }



}
