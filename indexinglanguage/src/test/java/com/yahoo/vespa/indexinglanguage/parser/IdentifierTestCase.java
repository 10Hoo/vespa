// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.indexinglanguage.parser;

import org.junit.Test;

import java.util.Arrays;
import java.util.List;

import static org.junit.Assert.assertEquals;

/**
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public class IdentifierTestCase {

    @Test
    public void requireThatThereAreNoReservedWords() throws ParseException {
        List<String> tokens = Arrays.asList("attribute",
                                            "base64_decode",
                                            "base64_encode",
                                            "clear_state",
                                            "compact_phrase",
                                            "create_if_non_existent",
                                            "echo",
                                            "exact",
                                            "flatten",
                                            "for_each",
                                            "get_field",
                                            "get_var",
                                            "guard",
                                            "hex_decode",
                                            "hex_encode",
                                            "host_name",
                                            "if",
                                            "index",
                                            "join",
                                            "linguistics",
                                            "lower_case",
                                            "ngram",
                                            "normalize",
                                            "now",
                                            "optimize_predicate",
                                            "predicate_to_raw",
                                            "put_symbol",
                                            "random",
                                            "raw_to_predicate",
                                            "remove_ctrl_chars",
                                            "remove_if_zero",
                                            "remove_so_si",
                                            "select_field",
                                            "set_language",
                                            "set_var",
                                            "split",
                                            "substring",
                                            "summary",
                                            "switch",
                                            "this",
                                            "tokenize",
                                            "to_array",
                                            "to_double",
                                            "to_float",
                                            "to_int",
                                            "to_long",
                                            "to_pos",
                                            "to_string",
                                            "to_wset",
                                            "trim",
                                            "zcurve");
        for (String str : tokens) {
            IndexingParser parser = new IndexingParser(new IndexingInput(str));
            assertEquals(str, parser.identifier());
        }
    }
}
