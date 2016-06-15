// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model.application.validation.change.search;

import com.yahoo.vespa.model.application.validation.ValidationOverrides;
import com.yahoo.vespa.model.application.validation.change.VespaConfigChangeAction;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

import static com.yahoo.vespa.model.application.validation.change.ConfigChangeTestUtils.newRefeedAction;

/**
 * Test validation of changes between a current and next document type used in a document database.
 *
 * @author  <a href="mailto:Tor.Egge@yahoo-inc.com">Tor Egge</a>
 * @since 2014-11-25
 */
public class DocumentTypeChangeValidatorTest {

    private static class Fixture extends ContentClusterFixture {
        DocumentTypeChangeValidator validator;

        public Fixture(String currentSd, String nextSd) throws Exception {
            super(currentSd, nextSd);
            validator = new DocumentTypeChangeValidator(currentDocType(), nextDocType());
        }

        @Override
        public List<VespaConfigChangeAction> validate() {
            return validator.validate(ValidationOverrides.empty());
        }

    }

    @Test
    public void requireThatFieldRemovalIsOK() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: summary }",
                                "field f2 type string { indexing: summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatSameDataTypeIsOK() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: summary }",
                                "field f1 type string { indexing: summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatDataTypeChangeIsNotOK() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: summary }",
                                "field f1 type int { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f1' changed: data type: 'string' -> 'int'"));
    }

    @Test
    public void requireThatAddingCollectionTypeIsNotOK() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: summary }",
                                "field f1 type array<string> { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f1' changed: data type: 'string' -> 'Array<string>'"));
    }


    @Test
    public void requireThatSameNestedDataTypeIsOK() throws Exception {
        Fixture f = new Fixture("field f1 type array<string> { indexing: summary }",
                                "field f1 type array<string> { indexing: summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatNestedDataTypeChangeIsNotOK() throws Exception {
        Fixture f = new Fixture("field f1 type array<string> { indexing: summary }",
                                "field f1 type array<int> { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f1' changed: data type: 'Array<string>' -> 'Array<int>'"));
    }

    @Test
    public void requireThatChangedCollectionTypeIsNotOK() throws Exception {
        Fixture f = new Fixture("field f1 type array<string> { indexing: summary }",
                                "field f1 type weightedset<string> { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f1' changed: data type: 'Array<string>' -> 'WeightedSet<string>'"));
    }

    @Test
    public void requireThatMultipleDataTypeChangesIsNotOK() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: summary } field f2 type int { indexing: summary }" ,
                                "field f2 type string { indexing: summary } field f1 type int { indexing: summary }");
        f.assertValidation(Arrays.asList(newRefeedAction("field-type-change",
                                                         ValidationOverrides.empty(),
                                                         "Field 'f1' changed: data type: 'string' -> 'int'"),
                                         newRefeedAction("field-type-change",
                                                         ValidationOverrides.empty(),
                                                         "Field 'f2' changed: data type: 'int' -> 'string'")));
    }

    @Test
    public void requireThatSameDataTypeInStructFieldIsOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type string {} } field f2 type s1 { indexing: summary }",
                                "struct s1 { field f1 type string {} } field f2 type s1 { indexing: summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatSameNestedDataTypeChangeInStructFieldIsOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type array<string> {} } field f2 type s1 { indexing: summary }",
                                "struct s1 { field f1 type array<string> {} } field f2 type s1 { indexing: summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatAddingFieldInStructFieldIsOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type string {} } field f3 type s1 { indexing: summary }",
                                "struct s1 { field f1 type string {} field f2 type int {} } field f3 type s1 { indexing: summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatRemovingFieldInStructFieldIsOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type string {} field f2 type int {} } field f3 type s1 { indexing: summary }",
                                "struct s1 { field f1 type string {} } field f3 type s1 { indexing: summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatDataTypeChangeInStructFieldIsNotOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type string {} } field f2 type s1 { indexing: summary }",
                                "struct s1 { field f1 type int {} } field f2 type s1 { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f2' changed: data type: 's1:{f1:string}' -> 's1:{f1:int}'"));
    }

    @Test
    public void requireThatNestedDataTypeChangeInStructFieldIsNotOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type array<string> {} } field f2 type s1 { indexing: summary }",
                                "struct s1 { field f1 type array<int> {} } field f2 type s1 { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f2' changed: data type: 's1:{f1:Array<string>}' -> 's1:{f1:Array<int>}'"));
    }

    @Test
    public void requireThatDataTypeChangeInNestedStructFieldIsNotOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type string {} } struct s2 { field f2 type s1 {} } field f3 type s2 { indexing: summary }",
                                "struct s1 { field f1 type int {} }    struct s2 { field f2 type s1 {} } field f3 type s2 { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f3' changed: data type: 's2:{s1:{f1:string}}' -> 's2:{s1:{f1:int}}'"));
    }

    @Test
    public void requireThatMultipleDataTypeChangesInStructFieldIsNotOK() throws Exception {
        Fixture f = new Fixture("struct s1 { field f1 type string {} field f2 type int {} } field f3 type s1 { indexing: summary }",
                                "struct s1 { field f1 type int {} field f2 type string {} } field f3 type s1 { indexing: summary }");
        f.assertValidation(newRefeedAction("field-type-change",
                                           ValidationOverrides.empty(),
                                           "Field 'f3' changed: data type: 's1:{f1:string,f2:int}' -> 's1:{f1:int,f2:string}'"));
    }

}
