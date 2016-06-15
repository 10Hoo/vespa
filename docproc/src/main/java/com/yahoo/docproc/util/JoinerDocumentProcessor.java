// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.docproc.util;

import com.yahoo.component.ComponentId;
import com.yahoo.document.DocumentOperation;
import com.yahoo.document.DocumentPut;
import com.yahoo.document.config.DocumentmanagerConfig;
import com.yahoo.config.docproc.SplitterJoinerDocumentProcessorConfig;
import com.yahoo.docproc.DocumentProcessor;
import com.yahoo.docproc.Processing;
import com.yahoo.document.Document;
import com.yahoo.document.DocumentTypeManager;
import com.yahoo.document.DocumentTypeManagerConfigurer;
import com.yahoo.document.datatypes.Array;
import com.yahoo.log.LogLevel;

import java.util.logging.Logger;

import static com.yahoo.docproc.util.SplitterDocumentProcessor.validate;
import static com.yahoo.docproc.util.SplitterDocumentProcessor.doProcessOuterDocument;

/**
 * @author <a href="mailto:einarmr@yahoo-inc.com">Einar M R Rosenvinge</a>
 */
public class JoinerDocumentProcessor extends DocumentProcessor {

    private static Logger log = Logger.getLogger(JoinerDocumentProcessor.class.getName());
    private String documentTypeName;
    private String arrayFieldName;
    private String contextFieldName;
    DocumentTypeManager manager;

    public JoinerDocumentProcessor(SplitterJoinerDocumentProcessorConfig cfg, DocumentmanagerConfig documentmanagerConfig) {
        super();
        this.documentTypeName = cfg.documentTypeName();
        this.arrayFieldName = cfg.arrayFieldName();
        this.contextFieldName = cfg.contextFieldName();
        manager = DocumentTypeManagerConfigurer.configureNewManager(documentmanagerConfig);
        validate(manager, documentTypeName, arrayFieldName);
    }

    @Override
    public Progress process(Processing processing) {
        if ( ! doProcessOuterDocument(processing.getVariable(contextFieldName), documentTypeName)) {
            return Progress.DONE;
        }

        DocumentPut outerDoc = (DocumentPut)processing.getVariable(contextFieldName);

        Array<Document> innerDocuments = (Array<Document>) outerDoc.getDocument().getFieldValue(arrayFieldName);

        if (innerDocuments == null) {
            innerDocuments = (Array<Document>) outerDoc.getDocument().getDataType().getField(arrayFieldName).getDataType().createFieldValue();
        }

        for (DocumentOperation op : processing.getDocumentOperations()) {
            if (op instanceof DocumentPut) {
                innerDocuments.add(((DocumentPut)op).getDocument());
            } else {
                log.log(LogLevel.DEBUG, "Skipping: " + op);
            }
        }
        processing.getDocumentOperations().clear();
        processing.getDocumentOperations().add(outerDoc);
        processing.removeVariable(contextFieldName);
        return Progress.DONE;
    }
}
