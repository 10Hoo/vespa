// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.json;

import com.fasterxml.jackson.core.JsonFactory;
import com.yahoo.document.ArrayDataType;
import com.yahoo.document.DataType;
import com.yahoo.document.DocumentType;
import com.yahoo.document.DocumentTypeManager;
import com.yahoo.document.DocumentUpdate;
import com.yahoo.document.Field;
import com.yahoo.document.MapDataType;
import com.yahoo.document.PositionDataType;
import com.yahoo.document.ReferenceDataType;
import com.yahoo.document.StructDataType;
import com.yahoo.document.TensorDataType;
import com.yahoo.document.WeightedSetDataType;
import com.yahoo.document.json.document.DocumentParser;
import com.yahoo.tensor.TensorType;
import com.yahoo.text.Utf8;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;

import static com.yahoo.test.json.JsonTestHelper.assertJsonEquals;
import static com.yahoo.test.json.JsonTestHelper.inputJson;

/**
 * @author Vegard Sjonfjell
 */
public class DocumentUpdateJsonSerializerTest {

    final static TensorType tensorType = new TensorType.Builder().mapped("x").mapped("y").build();
    final static DocumentTypeManager types = new DocumentTypeManager();
    final static JsonFactory parserFactory = new JsonFactory();
    final static DocumentType docType = new DocumentType("doctype");
    final static DocumentType refTargetDocType = new DocumentType("target_doctype");

    final static String DEFAULT_DOCUMENT_ID = "id:test:doctype::1";

    static {
        StructDataType myStruct = new StructDataType("my_struct");
        myStruct.addField(new Field("my_string_field", DataType.STRING));
        myStruct.addField(new Field("my_int_field", DataType.INT));
        types.registerDocumentType(refTargetDocType);

        docType.addField(new Field("string_field", DataType.STRING));
        docType.addField(new Field("int_field", DataType.INT));
        docType.addField(new Field("float_field", DataType.FLOAT));
        docType.addField(new Field("double_field", DataType.DOUBLE));
        docType.addField(new Field("byte_field", DataType.BYTE));
        docType.addField(new Field("tensor_field", new TensorDataType(tensorType)));
        docType.addField(new Field("reference_field", new ReferenceDataType(refTargetDocType, 777)));
        docType.addField(new Field("predicate_field", DataType.PREDICATE));
        docType.addField(new Field("raw_field", DataType.RAW));
        docType.addField(new Field("int_array", new ArrayDataType(DataType.INT)));
        docType.addField(new Field("string_array", new ArrayDataType(DataType.STRING)));
        docType.addField(new Field("int_set", new WeightedSetDataType(DataType.INT, true, true)));
        docType.addField(new Field("string_set", new WeightedSetDataType(DataType.STRING, true, true)));
        docType.addField(new Field("string_map", new MapDataType(DataType.STRING, DataType.STRING)));
        docType.addField(new Field("deep_map", new MapDataType(DataType.STRING, new MapDataType(DataType.STRING, DataType.STRING))));
        docType.addField(new Field("map_array", new MapDataType(DataType.STRING, new ArrayDataType(DataType.STRING))));
        docType.addField(new Field("map_struct", new MapDataType(DataType.STRING, myStruct)));
        docType.addField(new Field("singlepos_field", PositionDataType.INSTANCE));
        docType.addField(new Field("multipos_field", new ArrayDataType(PositionDataType.INSTANCE)));
        types.registerDocumentType(docType);
    }

    private static DocumentUpdate deSerializeDocumentUpdate(String jsonDoc, String docId) {
        final InputStream rawDoc = new ByteArrayInputStream(Utf8.toBytes(jsonDoc));
        JsonReader reader = new JsonReader(types, rawDoc, parserFactory);
        return (DocumentUpdate) reader.readSingleDocument(DocumentParser.SupportedOperation.UPDATE, docId);
    }

    private static String serializeDocumentUpdate(DocumentUpdate update) {
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        DocumentUpdateJsonSerializer serializer = new DocumentUpdateJsonSerializer(outputStream);
        serializer.serialize(update);

        try {
            return new String(outputStream.toByteArray(), "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    private static void deSerializeAndSerializeJsonAndMatch(String jsonDoc) {
        jsonDoc = jsonDoc.replaceFirst("DOCUMENT_ID", DEFAULT_DOCUMENT_ID);
        DocumentUpdate update = deSerializeDocumentUpdate(jsonDoc, DEFAULT_DOCUMENT_ID);
        assertJsonEquals(serializeDocumentUpdate(update), jsonDoc);
    }

    @Test
    public void testArithmeticUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'int_field': {",
                "            'increment': 3.0",
                "        },",
                "        'float_field': {",
                "            'decrement': 1.5",
                "        },",
                "        'double_field': {",
                "            'divide': 3.2",
                "        },",
                "        'byte_field': {",
                "            'multiply': 2.0",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testAssignSimpleTypes() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'int_field': {",
                "            'assign': 42",
                "        },",
                "        'float_field': {",
                "            'assign': 32.45",
                "        },",
                "        'double_field': {",
                "            'assign': 45.93",
                "        },",
                "        'string_field': {",
                "            'assign': \"My favorite string\"",
                "        },",
                "        'byte_field': {",
                "            'assign': 127",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testAssignWeightedSet() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'int_set': {",
                "            'assign': {",
                "                '123': 456,",
                "                '789': 101112",
                "            }",
                "        },",
                "        'string_set': {",
                "            'assign': {",
                "                'meow': 218478,",
                "                'slurp': 2123",
                "            }",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testAddUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'int_array': {",
                "            'add': [",
                "                123,",
                "                456,",
                "                789",
                "            ]",
                "        },",
                "        'string_array': {",
                "            'add': [",
                "                'bjarne',",
                "                'andrei',",
                "                'rich'",
                "            ]",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testRemoveUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'int_array': {",
                "            'remove': [",
                "                123,",
                "                789",
                "            ]",
                "        },",
                "        'string_array': {",
                "            'remove': [",
                "                'bjarne',",
                "                'rich'",
                "            ]",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testMatchUpdateArithmetic() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'int_array': {",
                "            'match': {",
                "                'element': 456,",
                "                'multiply': 8.0",
                "            }",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testMatchUpdateAssign() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "     'fields': {",
                "          'string_array': {",
                "               'match': {",
                "                   'element': 3,",
                "                   'assign': 'kjeks'",
                "               }",
                "          }",
                "     }",
                "}"
        ));
    }

    @Test
    public void testAssignTensor() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'tensor_field': {",
                "            'assign': {",
                "                'cells': [",
                "                    { 'address': { 'x': 'a', 'y': 'b' }, 'value': 2.0 },",
                "                    { 'address': { 'x': 'c', 'y': 'b' }, 'value': 3.0 }",
                "                ]",
                "            }",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void reference_field_id_can_be_update_assigned_non_empty_id() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'reference_field': {",
                "            'assign': 'id:ns:target_doctype::foo'",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void reference_field_id_can_be_update_assigned_empty_id() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'reference_field': {",
                "            'assign': ''",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testAssignPredicate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'predicate_field': {",
                "            'assign': 'foo in [bar]'",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testAssignRaw() {
                deSerializeAndSerializeJsonAndMatch(inputJson(
                        "{",
                        "    'update': 'DOCUMENT_ID',",
                        "    'fields': {",
                        "        'raw_field': {",
                        "            'assign': 'RG9uJ3QgYmVsaWV2ZSBoaXMgbGllcw==\\r\\n'",
                        "        }",
                        "    }",
                        "}"
                ));
    }

    @Test
    public void testAssignMap() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'string_map': {",
                "            'assign': { ",
                "                   'conversion gel': 'deadly',",
                "                   'repulsion gel': 'safe',",
                "                   'propulsion gel': 'insufficient data'",
                "              }",
                "         }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testSimultaneousFieldsAndFieldPathsUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "   'update': 'DOCUMENT_ID',",
                "   'fields': {",
                "       'singlepos_field': {",
                "           'assign': 'N60.222333;E10.12'",
                "       },",
                "       'deep_map{my_field}': {",
                "           'assign': {",
                "               'my_key': 'my value',",
                "               'new_key': 'new value'",
                "           }",
                "       }",
                "   }",
                "}"
        ));
    }

    @Test
    public void testAssignFieldPathUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "   'update': 'DOCUMENT_ID',",
                "   'fields': {",
                "       'deep_map{my_field}': {",
                "           'assign': {",
                "               'my_key': 'my value',",
                "               'new_key': 'new value'",
                "           }",
                "       },",
                "       'map_struct{my_key}': {",
                "           'assign': {",
                "               'my_string_field': 'Some string',",
                "               'my_int_field': 5",
                "           }",
                "       },",
                "       'map_struct{my_key}.my_int_field': {",
                "           'assign': 10",
                "       }",
                "   }",
                "}"
        ));
    }

    @Test
    public void testRemoveFieldPathUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "   'update': 'DOCUMENT_ID',",
                "   'fields': {",
                "       'int_array[5]': {",
                "           'remove': 0",
                "       }",
                "   }",
                "}"
        ));
    }

    @Test
    public void testAddFieldPathUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "   'update': 'DOCUMENT_ID',",
                "   'fields': {",
                "       'map_array{my_value}': {",
                "           'add': ['some', 'fancy', 'strings']",
                "       }",
                "   }",
                "}"
        ));
    }

    @Test
    public void testArithmeticFieldPathUpdate() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "   'update': 'DOCUMENT_ID',",
                "   'fields': {",
                "       'map_struct{my_key}.my_int_field': {",
                "           'increment': 5.0",
                "       },",
                "       'int_array[10]': {",
                "           'divide': 3.0",
                "       }",
                "   }",
                "}"
        ));
    }

    @Test
    public void testMultipleOperationsOnSingleFieldPath() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "   'update': 'DOCUMENT_ID',",
                "   'fields': {",
                "       'map_struct{my_key}': {",
                "           'assign': {",
                "               'my_string_field': 'Some string'",
                "           },",
                "           'remove': 0",
                "       }",
                "   }",
                "}"
        ));
    }

    @Test
    public void testAssignSinglePos() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'singlepos_field': {",
                "            'assign': 'N60.222333;E10.12'",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testAssignMultiPos() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'multipos_field': {",
                "            'assign': [ 'N0.0;E0.0', 'S1.1;W1.1', 'N10.2;W122.2' ]",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testClearField() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'fields': {",
                "        'int_field': {",
                "            'assign': null",
                "        },",
                "        'string_field': {",
                "            'assign': null",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testCreateIfNotExistTrue() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'create': true,",
                "    'fields': {",
                "        'int_field': {",
                "            'assign': 42",
                "        }",
                "    }",
                "}"
        ));
    }

    @Test
    public void testCreateIfNotExistFalse() {
        deSerializeAndSerializeJsonAndMatch(inputJson(
                "{",
                "    'update': 'DOCUMENT_ID',",
                "    'create': false,",
                "    'fields': {",
                "        'int_field': {",
                "            'assign': 42",
                "        }",
                "    }",
                "}"
        ));
    }
}
