// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.search.pagetemplates.engine;

import com.yahoo.search.result.ChainableComparator;
import com.yahoo.search.result.Hit;

import java.util.Comparator;

/**
 * @author <a href="mailto:bratseth@yahoo-inc.com">Jon Bratseth</a>
 */
class RelevanceComparator extends ChainableComparator {

    /**
     * Creates a relevance comparator, with an optional secondary comparator.
     * If the secondary is null, the intrinsic hit order is used as secondary.
     */
    public RelevanceComparator(Comparator<Hit> secondaryComparator) {
        super(secondaryComparator);
    }

    public @Override int compare(Hit h1,Hit h2) {
        int relevanceComparison=h2.getRelevance().compareTo(h1.getRelevance());
        if (relevanceComparison!=0) return relevanceComparison;

        return super.compare(h1,h2);
    }

}
