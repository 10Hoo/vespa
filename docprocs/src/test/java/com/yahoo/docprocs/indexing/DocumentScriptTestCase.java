// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.docprocs.indexing;

import com.yahoo.document.DataType;
import com.yahoo.document.Document;
import com.yahoo.document.DocumentType;
import com.yahoo.document.DocumentUpdate;
import com.yahoo.document.Field;
import com.yahoo.document.StructDataType;
import com.yahoo.document.annotation.SpanTree;
import com.yahoo.document.annotation.SpanTrees;
import com.yahoo.document.datatypes.Array;
import com.yahoo.document.datatypes.FieldValue;
import com.yahoo.document.datatypes.MapFieldValue;
import com.yahoo.document.datatypes.StringFieldValue;
import com.yahoo.document.datatypes.Struct;
import com.yahoo.document.datatypes.WeightedSet;
import com.yahoo.document.fieldpathupdate.AssignFieldPathUpdate;
import com.yahoo.document.update.FieldUpdate;
import com.yahoo.document.update.MapValueUpdate;
import com.yahoo.document.update.ValueUpdate;
import com.yahoo.vespa.indexinglanguage.AdapterFactory;
import com.yahoo.vespa.indexinglanguage.SimpleAdapterFactory;
import com.yahoo.vespa.indexinglanguage.expressions.Expression;
import com.yahoo.vespa.indexinglanguage.expressions.IndexExpression;
import com.yahoo.vespa.indexinglanguage.expressions.InputExpression;
import com.yahoo.vespa.indexinglanguage.expressions.StatementExpression;
import com.yahoo.vespa.indexinglanguage.parser.ParseException;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
@SuppressWarnings("unchecked")
public class DocumentScriptTestCase {

    private static final AdapterFactory ADAPTER_FACTORY = new SimpleAdapterFactory();

    @Test
    public void requireThatDocumentWithExtraFieldsThrow() throws ParseException {
        assertFail("Field 'extraField' is not part of the declared document type 'documentType'.",
                   newDocument(new StringFieldValue("foo"), new StringFieldValue("bar")));
        assertFail("Field 'extraField' is not part of the declared document type 'documentType'.",
                   newDocument(null, new StringFieldValue("bar")));
    }

    @Test
    public void requireThatFieldUpdateToExtraFieldsThrow() throws ParseException {
        assertFail("Field 'extraField' is not part of the declared document type 'documentType'.",
                   newFieldUpdate(new StringFieldValue("foo"), new StringFieldValue("bar")));
        assertFail("Field 'extraField' is not part of the declared document type 'documentType'.",
                   newFieldUpdate(null, new StringFieldValue("bar")));
    }

    @Test
    public void requireThatPathUpdateToExtraFieldsThrow() throws ParseException {
        assertFail("Field 'extraField' is not part of the declared document type 'documentType'.",
                   newPathUpdate(new StringFieldValue("foo"), new StringFieldValue("bar")));
        assertFail("Field 'extraField' is not part of the declared document type 'documentType'.",
                   newPathUpdate(null, new StringFieldValue("bar")));
    }

    @Test
    public void requireThatLinguisticsSpanTreeIsRemovedFromStringFields() {
        StringFieldValue in = newString(SpanTrees.LINGUISTICS, "mySpanTree");
        StringFieldValue out = (StringFieldValue)processDocument(in);
        assertSpanTrees(out, "mySpanTree");

        out = (StringFieldValue)processFieldUpdate(in).getValue();
        assertSpanTrees(out, "mySpanTree");

        out = (StringFieldValue)processPathUpdate(in).getValue();
        assertSpanTrees(out, "mySpanTree");
    }

    @Test
    public void requireThatLinguisticsSpanTreeIsRemovedFromArrayStringFields() {
        Array<StringFieldValue> in = new Array<>(DataType.getArray(DataType.STRING));
        in.add(newString(SpanTrees.LINGUISTICS, "mySpanTree"));

        Array<StringFieldValue> out = (Array<StringFieldValue>)processDocument(in);
        assertEquals(1, out.size());
        assertSpanTrees(out.get(0), "mySpanTree");

        out = (Array<StringFieldValue>)processFieldUpdate(in).getValue();
        assertEquals(1, out.size());
        assertSpanTrees(out.get(0), "mySpanTree");

        out = (Array<StringFieldValue>)processPathUpdate(in).getValue();
        assertEquals(1, out.size());
        assertSpanTrees(out.get(0), "mySpanTree");
    }

    @Test
    public void requireThatLinguisticsSpanTreeIsRemovedFromWsetStringFields() {
        WeightedSet<StringFieldValue> in = new WeightedSet<>(DataType.getWeightedSet(DataType.STRING));
        in.put(newString(SpanTrees.LINGUISTICS, "mySpanTree"), 69);

        WeightedSet<StringFieldValue> out = (WeightedSet<StringFieldValue>)processDocument(in);
        assertEquals(1, out.size());
        assertSpanTrees(out.keySet().iterator().next(), "mySpanTree");

        out = (WeightedSet<StringFieldValue>)processFieldUpdate(in).getValue();
        assertEquals(1, out.size());
        assertSpanTrees(out.keySet().iterator().next(), "mySpanTree");

        out = (WeightedSet<StringFieldValue>)processPathUpdate(in).getValue();
        assertEquals(1, out.size());
        assertSpanTrees(out.keySet().iterator().next(), "mySpanTree");
    }

    @Test
    public void requireThatLinguisticsSpanTreeIsRemovedFromMapStringStringFields() {
        MapFieldValue<StringFieldValue, StringFieldValue> in =
                new MapFieldValue<>(DataType.getMap(DataType.STRING, DataType.STRING));
        in.put(newString(SpanTrees.LINGUISTICS, "myKeySpanTree"),
               newString(SpanTrees.LINGUISTICS, "myValueSpanTree"));

        MapFieldValue<StringFieldValue, StringFieldValue> out;
        out = (MapFieldValue<StringFieldValue, StringFieldValue>)processDocument(in);
        assertEquals(1, out.size());
        assertSpanTrees(out.keySet().iterator().next(), "myKeySpanTree");
        assertSpanTrees(out.values().iterator().next(), "myValueSpanTree");

        out = (MapFieldValue<StringFieldValue, StringFieldValue>)processFieldUpdate(in).getValue();
        assertEquals(1, out.size());
        assertSpanTrees(out.keySet().iterator().next(), "myKeySpanTree");
        assertSpanTrees(out.values().iterator().next(), "myValueSpanTree");

        out = (MapFieldValue<StringFieldValue, StringFieldValue>)processPathUpdate(in).getValue();
        assertEquals(1, out.size());
        assertSpanTrees(out.keySet().iterator().next(), "myKeySpanTree");
        assertSpanTrees(out.values().iterator().next(), "myValueSpanTree");
    }

    @Test
    public void requireThatLinguisticsSpanTreeIsRemovedFromStructStringFields() {
        StructDataType structType = new StructDataType("myStruct");
        structType.addField(new Field("myString", DataType.STRING));
        Struct in = new Struct(structType);
        in.setFieldValue("myString", newString(SpanTrees.LINGUISTICS, "mySpanTree"));

        Struct out = (Struct)processDocument(in);
        assertSpanTrees(out.getFieldValue("myString"), "mySpanTree");

        StringFieldValue str = (StringFieldValue)((MapValueUpdate)processFieldUpdate(in)).getUpdate().getValue();
        assertSpanTrees(str, "mySpanTree");

        str = (StringFieldValue)((MapValueUpdate)processFieldUpdate(in)).getUpdate().getValue();
        assertSpanTrees(str, "mySpanTree");
    }

    private static FieldValue processDocument(FieldValue fieldValue) {
        DocumentType docType = new DocumentType("myDocumentType");
        docType.addField("myField", fieldValue.getDataType());
        Document doc = new Document(docType, "doc:scheme:");
        doc.setFieldValue("myField", fieldValue.clone());
        doc = newScript(docType).execute(ADAPTER_FACTORY, doc);
        return doc.getFieldValue("myField");
    }

    private static ValueUpdate<?> processFieldUpdate(FieldValue fieldValue) {
        DocumentType docType = new DocumentType("myDocumentType");
        docType.addField("myField", fieldValue.getDataType());
        DocumentUpdate update = new DocumentUpdate(docType, "doc:scheme:");
        update.addFieldUpdate(FieldUpdate.createAssign(docType.getField("myField"), fieldValue));
        update = newScript(docType).execute(ADAPTER_FACTORY, update);
        return update.getFieldUpdate("myField").getValueUpdate(0);
    }

    private static ValueUpdate<?> processPathUpdate(FieldValue fieldValue) {
        DocumentType docType = new DocumentType("myDocumentType");
        docType.addField("myField", fieldValue.getDataType());
        DocumentUpdate update = new DocumentUpdate(docType, "doc:scheme:");
        update.addFieldPathUpdate(new AssignFieldPathUpdate(docType, "myField", fieldValue));
        update = newScript(docType).execute(ADAPTER_FACTORY, update);
        return update.getFieldUpdate("myField").getValueUpdate(0);
    }

    private static DocumentScript newScript(DocumentType docType) {
        String fieldName = docType.getFields().iterator().next().getName();
        return new DocumentScript(docType.getName(), Arrays.asList(fieldName),
                                  new StatementExpression(new InputExpression(fieldName),
                                                          new IndexExpression(fieldName)));
    }

    private static StringFieldValue newString(String... spanTrees) {
        StringFieldValue ret = new StringFieldValue("foo");
        for (String spanTree : spanTrees) {
            ret.setSpanTree(new SpanTree(spanTree));
        }
        return ret;
    }

    private static void assertSpanTrees(FieldValue actual, String... expectedSpanTrees) {
        assertTrue(actual instanceof StringFieldValue);
        StringFieldValue str = (StringFieldValue)actual;
        assertEquals(new ArrayList<>(Arrays.asList(expectedSpanTrees)),
                     new ArrayList<>(str.getSpanTreeMap().keySet()));
    }

    private static DocumentType newDocumentType() {
        DocumentType type = new DocumentType("documentType");
        type.addField("documentField", DataType.STRING);
        type.addField("extraField", DataType.STRING);
        return type;
    }

    private static Document newDocument(FieldValue documentFieldValue, FieldValue extraFieldValue) {
        Document document = new Document(newDocumentType(), "doc:scheme:");
        if (documentFieldValue != null) {
            document.setFieldValue("documentField", documentFieldValue);
        }
        if (extraFieldValue != null) {
            document.setFieldValue("extraField", extraFieldValue);
        }
        return document;
    }

    private static DocumentUpdate newFieldUpdate(FieldValue documentFieldValue, FieldValue extraFieldValue) {
        DocumentType type = newDocumentType();
        DocumentUpdate update = new DocumentUpdate(type, "doc:scheme:");
        if (documentFieldValue != null) {
            update.addFieldUpdate(FieldUpdate.createAssign(type.getField("documentField"), documentFieldValue));
        }
        if (extraFieldValue != null) {
            update.addFieldUpdate(FieldUpdate.createAssign(type.getField("extraField"), extraFieldValue));
        }
        return update;
    }

    private static DocumentUpdate newPathUpdate(FieldValue documentFieldValue, FieldValue extraFieldValue) {
        DocumentType type = newDocumentType();
        DocumentUpdate update = new DocumentUpdate(type, "doc:scheme:");
        if (documentFieldValue != null) {
            update.addFieldPathUpdate(new AssignFieldPathUpdate(type, "documentField", documentFieldValue));
        }
        if (extraFieldValue != null) {
            update.addFieldPathUpdate(new AssignFieldPathUpdate(type, "extraField", extraFieldValue));
        }
        return update;
    }

    private static void assertFail(String expectedException, Document document) throws ParseException {
        try {
            execute(document);
            fail();
        } catch (IllegalArgumentException e) {
            assertEquals(expectedException, e.getMessage());
        }
    }

    private static void assertFail(String expectedException, DocumentUpdate update) throws ParseException {
        try {
            execute(update);
            fail();
        } catch (IllegalArgumentException e) {
            assertEquals(expectedException, e.getMessage());
        }
    }

    private static Document execute(Document document) throws ParseException {
        return newScript().execute(new SimpleAdapterFactory(), document);
    }

    private static DocumentUpdate execute(DocumentUpdate update) throws ParseException {
        return newScript().execute(new SimpleAdapterFactory(), update);
    }

    private static DocumentScript newScript() throws ParseException {
        return new DocumentScript("documentType", Arrays.asList("documentField"),
                                  Expression.fromString("input documentField | index documentField"));
    }
}
