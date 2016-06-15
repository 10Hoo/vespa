// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Unit tests for predicatefieldvalue.

#include <vespa/log/log.h>
LOG_SETUP("predicatefieldvalue_test");
#include <vespa/fastos/fastos.h>

#include <vespa/document/predicate/predicate.h>
#include <vespa/vespalib/data/slime/slime.h>
#include <vespa/vespalib/testkit/testapp.h>

#include <vespa/document/datatype/datatype.h>
#include <vespa/document/fieldvalue/predicatefieldvalue.h>
#include <vespa/document/predicate/predicate_slime_builder.h>
#include <sstream>
#include <string>

using std::ostringstream;
using std::string;
using vespalib::Slime;
using namespace document;

namespace {

void verifyEqual(const FieldValue & a, const FieldValue & b) {
    ostringstream o1;
    a.print(o1, false, "");
    ostringstream o2;
    b.print(o2, false, "");
    ASSERT_EQUAL(o1.str(), o2.str());
}

TEST("require that PredicateFieldValue can be cloned, assigned, and operator=") {
    PredicateSlimeBuilder builder;
    builder.neg().feature("foo").value("bar").value("baz");
    PredicateFieldValue val(builder.build());

    FieldValue::UP val2(val.clone());
    verifyEqual(val, *val2);

    PredicateFieldValue assigned;
    assigned.assign(val);
    verifyEqual(val, assigned);

    PredicateFieldValue operatorAssigned;
    operatorAssigned = val;
    verifyEqual(val, operatorAssigned);
}

TEST("require that PredicateFieldValue can be created from datatype") {
    FieldValue::UP val = DataType::PREDICATE->createFieldValue();
    ASSERT_TRUE(dynamic_cast<PredicateFieldValue *>(val.get()));
}

TEST("require that PredicateFieldValue can be cloned") {
    PredicateSlimeBuilder builder;
    builder.neg().feature("foo").value("bar").value("baz");
    PredicateFieldValue val(builder.build());
    FieldValue::UP val2(val.clone());
    ostringstream o1;
    val.print(o1, false, "");
    ostringstream o2;
    val2->print(o2, false, "");
    ASSERT_EQUAL(o1.str(), o2.str());
}


}  // namespace

TEST_MAIN() { TEST_RUN_ALL(); }
