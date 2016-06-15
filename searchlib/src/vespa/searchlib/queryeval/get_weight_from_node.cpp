// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".get_weight_from_node");

#include "get_weight_from_node.h"
#include <vespa/searchlib/query/tree/intermediatenodes.h>
#include <vespa/searchlib/query/tree/node.h>
#include <vespa/searchlib/query/tree/simplequery.h>
#include <vespa/searchlib/query/tree/templatetermvisitor.h>
#include <vespa/searchlib/query/tree/termnodes.h>

using search::query::Node;
using search::query::SimpleQueryNodeTypes;
using search::query::TemplateTermVisitor;
using search::query::Weight;

namespace search {
namespace queryeval {
namespace {

struct WeightExtractor : public TemplateTermVisitor<WeightExtractor,
                                                    SimpleQueryNodeTypes> {
    Weight weight;

    WeightExtractor() : weight(0) {}

    template <class TermType> void visitTerm(TermType &n) {
        weight = n.getWeight();
    }

    // Treat Equiv nodes as terms.
    virtual void visit(search::query::Equiv &n) { visitTerm(n); }
};

} // namespace search::queryeval::<unnamed>

Weight
getWeightFromNode(const Node &node)
{
    WeightExtractor extractor;
    const_cast<Node &>(node).accept(extractor);
    return extractor.weight;
}

} // namespace search::queryeval
} // namespace search
