// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model.application.validation.change.search;

import com.yahoo.vespa.model.application.validation.ValidationOverrides;
import com.yahoo.vespa.model.application.validation.change.VespaConfigChangeAction;
import org.junit.Test;

import java.util.List;

import static com.yahoo.vespa.model.application.validation.change.ConfigChangeTestUtils.newRefeedAction;
import static com.yahoo.vespa.model.application.validation.change.ConfigChangeTestUtils.newRestartAction;

public class AttributeChangeValidatorTest {

    private static class Fixture extends ContentClusterFixture {
        AttributeChangeValidator validator;

        public Fixture(String currentSd, String nextSd) throws Exception {
            super(currentSd, nextSd);
            validator = new AttributeChangeValidator(currentDb().getDerivedConfiguration().getAttributeFields(),
                    currentDb().getDerivedConfiguration().getIndexSchema(),
                    currentDocType(),
                    nextDb().getDerivedConfiguration().getAttributeFields(),
                    nextDb().getDerivedConfiguration().getIndexSchema(),
                    nextDocType());
        }

        @Override
        public List<VespaConfigChangeAction> validate() {
            return validator.validate(ValidationOverrides.empty());
        }

    }

    @Test
    public void requireThatAddingAttributeAspectRequireRestart() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: summary }",
                "field f1 type string { indexing: attribute | summary }");
        f.assertValidation(newRestartAction(
                "Field 'f1' changed: add attribute aspect"));
    }

    @Test
    public void requireThatRemovingAttributeAspectRequireRestart() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: attribute | summary }",
                "field f1 type string { indexing: summary }");
        f.assertValidation(newRestartAction(
                "Field 'f1' changed: remove attribute aspect"));
    }

    @Test
    public void requireThatAddingAttributeFieldIsOk() throws Exception {
        Fixture f = new Fixture("", "field f1 type string { indexing: attribute | summary \n attribute: fast-search }");
        f.assertValidation();
    }

    @Test
    public void requireThatRemovingAttributeFieldIsOk() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: attribute | summary }", "");
        f.assertValidation();
    }

    @Test
    public void requireThatChangingFastSearchRequireRestart() throws Exception {
        new Fixture("field f1 type string { indexing: attribute }",
                "field f1 type string { indexing: attribute \n attribute: fast-search }").
                assertValidation(newRestartAction(
                        "Field 'f1' changed: add attribute 'fast-search'"));
    }

    @Test
    public void requireThatChangingFastAccessRequireRestart() throws Exception {
        new Fixture("field f1 type string { indexing: attribute \n attribute: fast-access }",
                "field f1 type string { indexing: attribute }").
                assertValidation(newRestartAction(
                        "Field 'f1' changed: remove attribute 'fast-access'"));
    }

    @Test
    public void requireThatChangingHugeRequireRestart() throws Exception {
        new Fixture("field f1 type string { indexing: attribute }",
                "field f1 type string { indexing: attribute \n attribute: huge }").
                assertValidation(newRestartAction(
                        "Field 'f1' changed: add attribute 'huge'"));
    }

    @Test
    public void requireThatChangingDensePostingListThresholdRequireRestart() throws Exception {
        new Fixture(
                "field f1 type predicate { indexing: attribute \n index { arity: 8 \n dense-posting-list-threshold: 0.2 } }",
                "field f1 type predicate { indexing: attribute \n index { arity: 8 \n dense-posting-list-threshold: 0.4 } }").
                assertValidation(newRestartAction(
                        "Field 'f1' changed: change property 'dense-posting-list-threshold' from '0.2' to '0.4'"));
    }

    @Test
    public void requireThatRemovingAttributeAspectFromIndexFieldIsOk() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: index | attribute }",
                "field f1 type string { indexing: index }");
        f.assertValidation();
    }

    @Test
    public void requireThatRemovingAttributeAspectFromIndexAndSummaryFieldIsOk() throws Exception {
        Fixture f = new Fixture("field f1 type string { indexing: index | attribute | summary }",
                "field f1 type string { indexing: index | summary }");
        f.assertValidation();
    }

    @Test
    public void requireThatChangingTensorTypeOfTensorFieldRequiresRefeed() throws Exception {
        new Fixture(
                "field f1 type tensor { indexing: attribute \n attribute: tensor(x[100]) }",
                "field f1 type tensor { indexing: attribute \n attribute: tensor(y[]) }")
                .assertValidation(newRefeedAction(
                        "tensor-type-change",
                        ValidationOverrides.empty(),
                        "Field 'f1' changed: tensor type: 'tensor(x[100])' -> 'tensor(y[])'"));
    }

    @Test
    public void requireThatNotChangingTensorTypeOfTensorFieldIsOk() throws Exception {
        new Fixture(
                "field f1 type tensor { indexing: attribute \n attribute: tensor(x[104], y[52]) }",
                "field f1 type tensor { indexing: attribute \n attribute: tensor(x[104], y[52]) }")
                .assertValidation();
    }




}
