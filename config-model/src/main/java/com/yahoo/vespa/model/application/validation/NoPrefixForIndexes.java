// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model.application.validation;

import com.yahoo.config.model.deploy.DeployState;
import com.yahoo.searchdefinition.document.Matching;
import com.yahoo.searchdefinition.document.SDField;
import com.yahoo.searchdefinition.Index;
import com.yahoo.searchdefinition.Search;
import com.yahoo.searchdefinition.derived.DerivedConfiguration;
import com.yahoo.vespa.model.VespaModel;
import com.yahoo.vespa.model.search.AbstractSearchCluster;
import com.yahoo.vespa.model.search.DocumentDatabase;
import com.yahoo.vespa.model.search.IndexedSearchCluster;

import java.util.Map;

/**
 * match:prefix for indexed fields not supported
 * @author vegardh
 *
 */
public class NoPrefixForIndexes extends Validator {

    @Override
    public void validate(VespaModel model, DeployState deployState) {
        for (AbstractSearchCluster cluster : model.getSearchClusters()) {
            if (cluster instanceof IndexedSearchCluster) {
                IndexedSearchCluster sc = (IndexedSearchCluster) cluster;
                for (DocumentDatabase docDb : sc.getDocumentDbs()) {
                    DerivedConfiguration sdConfig = docDb.getDerivedConfiguration();
                    Search search = sdConfig.getSearch();
                    for (SDField field : search.allFieldsList()) {
                        if (field.doesIndexing()) {
                            //if (!field.getIndexTo().isEmpty() && !field.getIndexTo().contains(field.getName())) continue;
                            if (field.getMatching().getAlgorithm().equals(Matching.Algorithm.PREFIX)) {
                                failField(search, field);
                            }
                            for (Map.Entry<String, Index> e : field.getIndices().entrySet()) {
                                if (e.getValue().isPrefix()) {
                                    failField(search, field);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    private void failField(Search search, SDField field) {
        throw new IllegalArgumentException("For search '" + search.getName() + "', field '" + field.getName() +
                                           "': match/index:prefix is not supported for indexes.");
    }
}