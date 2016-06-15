// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model.application.validation;

import java.util.List;
import java.util.logging.Level;

import com.yahoo.config.model.deploy.DeployState;
import com.yahoo.searchdefinition.document.Matching;
import com.yahoo.config.application.api.DeployLogger;
import com.yahoo.document.NumericDataType;
import com.yahoo.searchdefinition.document.SDField;
import com.yahoo.vespa.model.VespaModel;
import com.yahoo.vespa.model.search.AbstractSearchCluster;
import com.yahoo.vespa.model.search.SearchCluster;


/**
 * Validates streaming mode
 */
public class StreamingValidator extends Validator {

    @Override
    public void validate(VespaModel model, DeployState deployState) {
        List<AbstractSearchCluster> searchClusters = model.getSearchClusters();
        for (AbstractSearchCluster cluster : searchClusters) {
            if (!cluster.isStreaming()) {
                continue;
            }
            SearchCluster sc = (SearchCluster) cluster;
            warnStreamingAttributes(sc, deployState.getDeployLogger());
            warnStreamingGramMatching(sc, deployState.getDeployLogger());
        }
    }

    private void warnStreamingGramMatching(SearchCluster sc, DeployLogger logger) {
        if (sc.getSdConfig() != null) {
            for (SDField sd : sc.getSdConfig().getSearch().allFieldsList()) {
                if (sd.getMatching().getType().equals(Matching.Type.GRAM)) {
                    logger.log(Level.WARNING, "For streaming search cluster '" + sc.getClusterName() +
                            "', SD field '" + sd.getName() + "': n-gram matching is not supported for streaming search.");
                }
            }
        }
    }

    /**
     * Warn if one or more attributes are defined in a streaming search cluster SD.
     *
     * @param sc     a search cluster to be checked for attributes in streaming search
     * @param logger a DeployLogger
     */
    private void warnStreamingAttributes(SearchCluster sc, DeployLogger logger) {
        if (sc.getSdConfig() != null) {
            for (SDField sd : sc.getSdConfig().getSearch().allFieldsList()) {
                if (sd.doesAttributing()) {
                    warnStreamingAttribute(sc, sd, logger);
                }
            }
        }
    }

    private void warnStreamingAttribute(SearchCluster sc, SDField sd, DeployLogger logger) {
        // If the field is numeric, we can't print this, because we may have converted the field to
        // attribute indexing ourselves (IntegerIndex2Attribute)
        if (sd.getDataType() instanceof NumericDataType) return;
        logger.log(Level.WARNING, "For streaming search cluster '" + sc.getClusterName() +
                "', SD field '" + sd.getName() + "': 'attribute' has same match semantics as 'index'.");
    }
}