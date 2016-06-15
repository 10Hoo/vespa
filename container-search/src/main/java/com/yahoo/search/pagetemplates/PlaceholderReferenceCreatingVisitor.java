// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.search.pagetemplates;

import com.yahoo.search.pagetemplates.model.*;

import java.util.HashMap;
import java.util.Map;

/**
 * Creates references from all placeholders to the choices which resolves them.
 * If a placeholder is encountered which is not resolved by any choice, an IllegalArgumentException is thrown.
 *
 * @author <a href="mailto:bratseth@yahoo-inc.com">Jon Bratseth</a>
 */
class PlaceholderReferenceCreatingVisitor extends PageTemplateVisitor {

    private Map<String, MapChoice> placeholderIdToChoice=new HashMap<>();

    public PlaceholderReferenceCreatingVisitor(Map<String, MapChoice> placeholderIdToChoice) {
        this.placeholderIdToChoice=placeholderIdToChoice;
    }

    public @Override void visit(Placeholder placeholder) {
        MapChoice choice=placeholderIdToChoice.get(placeholder.getId());
        if (choice==null)
            throw new IllegalArgumentException(placeholder + " is not referenced by any choice");
        placeholder.setValueContainer(choice);
    }

}
