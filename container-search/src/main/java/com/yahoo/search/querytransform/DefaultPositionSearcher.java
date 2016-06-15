// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.search.querytransform;

import static com.yahoo.prelude.searcher.PosSearcher.POSITION_PARSING;

import com.yahoo.component.chain.dependencies.After;
import com.yahoo.component.chain.dependencies.Before;
import com.yahoo.prelude.IndexFacts;
import com.yahoo.prelude.Location;
import com.yahoo.search.Query;
import com.yahoo.search.Searcher;
import com.yahoo.search.searchchain.Execution;
import com.yahoo.search.searchchain.PhaseNames;

import java.util.List;
import java.util.Set;

/**
 * If default position has not been set, it will be set here.
 *
 * @author <a href="mailto:balder@yahoo-inc.com">Henning Baldersheim</a>
 */
@After({PhaseNames.RAW_QUERY, POSITION_PARSING})
@Before(PhaseNames.TRANSFORMED_QUERY)
public class DefaultPositionSearcher extends Searcher {

    @Override
    public com.yahoo.search.Result search(Query query, Execution execution) {
        Location location = query.getRanking().getLocation();
        if (location != null && (location.getAttribute() == null)) {
            IndexFacts facts = execution.context().getIndexFacts();
            List<String> search = facts.newSession(query.getModel().getSources(), query.getModel().getRestrict()).documentTypes();

            for (String sd : search) {
                String defaultPosition = facts.getDefaultPosition(sd);
                if (defaultPosition != null) {
                    location.setAttribute(defaultPosition);
                }
            }
            if (location.getAttribute() == null) {
                location.setAttribute(facts.getDefaultPosition(null));
            }
        }
        return execution.search(query);
    }

}
