// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchdefinition.processing;

import com.yahoo.config.application.api.DeployLogger;
import com.yahoo.searchdefinition.RankProfileRegistry;
import com.yahoo.searchdefinition.document.Attribute;
import com.yahoo.document.WeightedSetDataType;
import com.yahoo.searchdefinition.Search;
import com.yahoo.vespa.documentmodel.DocumentSummary;
import com.yahoo.vespa.documentmodel.SummaryField;
import com.yahoo.vespa.documentmodel.SummaryTransform;
import com.yahoo.vespa.model.container.search.QueryProfiles;

/**
 * Ensure that summary field transforms for fields having the same name
 * are consistent across summary classes
 *
 * @author bratseth
 */
public class SummaryConsistency extends Processor {

    public SummaryConsistency(Search search, DeployLogger deployLogger, RankProfileRegistry rankProfileRegistry, QueryProfiles queryProfiles) {
        super(search, deployLogger, rankProfileRegistry, queryProfiles);
    }

    @Override
    public void process() {
        for (DocumentSummary summary : search.getSummaries().values()) {
            if (summary.getName().equals("default")) continue;
            for (SummaryField summaryField : summary.getSummaryFields() ) {
                assertConsistency(summaryField,search);
                makeAttributeTransformIfAppropriate(summaryField,search);
            }
        }
    }

    /** If the source is an attribute, make this use the attribute transform */
    private void makeAttributeTransformIfAppropriate(SummaryField summaryField,Search search) {
        if (summaryField.getTransform()!=SummaryTransform.NONE) return;
        Attribute attribute=search.getAttribute(summaryField.getSingleSource());
        if (attribute==null) return;
        summaryField.setTransform(SummaryTransform.ATTRIBUTE);
    }

    private void assertConsistency(SummaryField summaryField,Search search) {
        SummaryField existingDefault=search.getSummary("default").getSummaryField(summaryField.getName()); // Compare to default
        if (existingDefault!=null) {
            assertConsistentTypes(existingDefault,summaryField);
            makeConsistentWithDefaultOrThrow(existingDefault,summaryField);
        }
        else {
            // If no default, compare to whichever definition of the field
            SummaryField existing=search.getExplicitSummaryField(summaryField.getName());
            if (existing==null) return;
            assertConsistentTypes(existing,summaryField);
            makeConsistentOrThrow(existing,summaryField,search);
        }
    }

    private void assertConsistentTypes(SummaryField field1,SummaryField field2) {
        if (field1.getDataType() instanceof WeightedSetDataType && field2.getDataType() instanceof WeightedSetDataType &&
            ((WeightedSetDataType)field1.getDataType()).getNestedType().equals(((WeightedSetDataType)field2.getDataType()).getNestedType()))
            return; // Disregard create-if-nonexistent and create-if-zero distinction
        if ( ! field1.getDataType().equals(field2.getDataType()))
            throw new IllegalArgumentException(field1.toLocateString() + " is inconsistent with " + field2.toLocateString() + ": All declarations of the same summary field must have the same type");
    }

    private void makeConsistentOrThrow(SummaryField field1, SummaryField field2,Search search) {
        if (field2.getTransform()==SummaryTransform.ATTRIBUTE && field1.getTransform()==SummaryTransform.NONE) {
            Attribute attribute=search.getAttribute(field1.getName());
            if (attribute != null) {
                field1.setTransform(SummaryTransform.ATTRIBUTE);
            }
        }

        if (field2.getTransform().equals(SummaryTransform.NONE)) {
            field2.setTransform(field1.getTransform());
        }
        else { // New field sets an explicit transform - must be the same
            assertEqualTransform(field1,field2);
        }
    }
    private void makeConsistentWithDefaultOrThrow(SummaryField defaultField,SummaryField newField) {
        if (newField.getTransform().equals(SummaryTransform.NONE)) {
            newField.setTransform(defaultField.getTransform());
        }
        else { // New field sets an explicit transform - must be the same
            assertEqualTransform(defaultField,newField);
        }
    }


    private void assertEqualTransform(SummaryField field1,SummaryField field2) {
        if (!field2.getTransform().equals(field1.getTransform())) {
            throw new IllegalArgumentException("Conflicting summary transforms. " + field2 +" is already defined as " +
                                       field1 + ". A field with the same name " +
                                       "can not have different transforms in different summary classes");
        }
    }


}
