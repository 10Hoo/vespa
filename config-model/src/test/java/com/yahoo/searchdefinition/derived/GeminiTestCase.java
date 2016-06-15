// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchdefinition.derived;

import com.yahoo.searchdefinition.parser.ParseException;
import org.junit.Test;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * @author bratseth
 */
public class GeminiTestCase extends AbstractExportingTestCase {

    @Test
    public void testRanking2() throws IOException, ParseException {
        DerivedConfiguration c = assertCorrectDeriving("gemini2");
        RawRankProfile p = c.getRankProfileList().getRankProfile("test");
        Map<String, String> ranking = removePartKeySuffixes(p.configProperties());
        assertEquals("attribute(right)", resolve(lookup("toplevel", ranking), ranking));
    }

    private Map<String, String> removePartKeySuffixes(Map<String, Object> p) {
        Map<String, String> pWithoutSuffixes = new HashMap<>();
        for (Map.Entry<String, Object> entry : p.entrySet())
            pWithoutSuffixes.put(removePartSuffix(entry.getKey()), entry.getValue().toString());
        return pWithoutSuffixes;
    }

    private String removePartSuffix(String s) {
        int partIndex = s.indexOf(".part");
        if (partIndex <= 0) return s;
        return s.substring(0, partIndex);
    }

    /**
     * Recurively resolves references to other ranking expressions - rankingExpression(name) -
     * and replaces the reference by the expression
     */
    private String resolve(String expression, Map<String, String> ranking) {
        int referenceStartIndex;
        while ((referenceStartIndex = expression.indexOf("rankingExpression(")) >= 0) {
            int referenceEndIndex = expression.indexOf(")", referenceStartIndex);
            expression = expression.substring(0, referenceStartIndex) +
                         resolve(lookup(expression.substring(referenceStartIndex + "rankingExpression(".length(), referenceEndIndex), ranking), ranking) +
                         expression.substring(referenceEndIndex + 1);
        }
        return expression;
    }

    private String lookup(String expressionName, Map<String, String> ranking) {
        String value = ranking.get("rankingExpression(" + expressionName + ").rankingScript");
        if (value == null) {
            System.out.println("Warning: No expression found for " + expressionName);
            return expressionName;
        }
        return value;
    }

}
