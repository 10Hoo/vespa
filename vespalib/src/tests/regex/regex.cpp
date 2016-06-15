// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/vespalib/testkit/test_kit.h>

#include <vespa/vespalib/util/regexp.h>
#include <vespa/vespalib/util/exception.h>

using namespace vespalib;

TEST("require that empty expression works as expected") {
    Regexp empty("");
    EXPECT_TRUE(empty.match(""));
    EXPECT_TRUE(empty.match("foo"));
    EXPECT_TRUE(empty.match("bar"));
}

TEST("require that substring expression works as expected") {
    Regexp re("foo");
    EXPECT_TRUE(re.match("foo"));
    EXPECT_TRUE(re.match("afoob"));
    EXPECT_TRUE(re.match("foo foo"));
    EXPECT_FALSE(re.match("bar"));
    EXPECT_FALSE(re.match("fobaroo"));
}

TEST("require that it is default case sentive") {
    Regexp re("foo");
    EXPECT_TRUE(re.match("foo"));
    EXPECT_FALSE(re.match("fOo"));
}

TEST("require that it is case insentive") {
    Regexp re("foo", Regexp::Flags().enableICASE());
    EXPECT_TRUE(re.match("foo"));
    EXPECT_TRUE(re.match("fOo"));
}

TEST("require that invalid expression fails compilation") {
    try {
        Regexp bad("[unbalanced");
        EXPECT_TRUE(false);
    } catch (Exception& e) {
        EXPECT_TRUE(true);
        fprintf(stderr, "got: %s\n", e.getMessage().c_str());
    }
}

TEST("require that replace works") {
    Regexp rep("foo");
    EXPECT_EQUAL(vespalib::string(""), rep.replace("", "bc"));
    EXPECT_EQUAL(vespalib::string("a"), rep.replace("a", "bc"));
    EXPECT_EQUAL(vespalib::string("bc"), rep.replace("foo", "bc"));
    EXPECT_EQUAL(vespalib::string("abc"), rep.replace("afoo", "bc"));
    EXPECT_EQUAL(vespalib::string("abcd"), rep.replace("afood", "bc"));
    EXPECT_EQUAL(vespalib::string("abcdbc"), rep.replace("afoodfoo", "bc"));
    EXPECT_EQUAL(vespalib::string("abcdbcbc"), rep.replace("afoodfoofoo", "bc"));
}

TEST("require that prefix detection works") {
    EXPECT_EQUAL("", Regexp::get_prefix(""));
    EXPECT_EQUAL("", Regexp::get_prefix("foo"));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo"));
    EXPECT_EQUAL("", Regexp::get_prefix("^foo|bar"));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo$"));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo[a-z]"));
    EXPECT_EQUAL("fo", Regexp::get_prefix("^foo{0,1}"));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo."));
    EXPECT_EQUAL("fo", Regexp::get_prefix("^foo*"));
    EXPECT_EQUAL("fo", Regexp::get_prefix("^foo?"));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo+"));
}

TEST("require that prefix detection sometimes underestimates the prefix size") {
    EXPECT_EQUAL("", Regexp::get_prefix("^^foo"));
    EXPECT_EQUAL("", Regexp::get_prefix("^foo(bar|baz)"));
    EXPECT_EQUAL("fo", Regexp::get_prefix("^foo{1,2}"));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo\\."));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo(bar)"));
    EXPECT_EQUAL("", Regexp::get_prefix("(^foo)"));
    EXPECT_EQUAL("", Regexp::get_prefix("^(foo)"));
    EXPECT_EQUAL("foo", Regexp::get_prefix("^foo[a]"));
    EXPECT_EQUAL("", Regexp::get_prefix("^foo|^foobar"));
}

const vespalib::string special("^|()[]{}.*?+\\$");

struct ExprFixture {
    std::vector<vespalib::string> expressions;
    ExprFixture() {
        expressions.push_back(special);
        for (char c: special) {
            expressions.push_back(vespalib::string(&c, 1));
        }
        expressions.push_back("abc");
        expressions.push_back("[:digit:]");
    }
};

TEST_F("require that regexp can be made from prefix string", ExprFixture()) {
    for (vespalib::string str: f1.expressions) {
        Regexp re(Regexp::make_from_prefix(str));
        EXPECT_TRUE(re.match(str));
        EXPECT_TRUE(re.match(str + "foo"));
        EXPECT_FALSE(re.match("foo" + str));
        EXPECT_FALSE(re.match("foo" + str + "bar"));
    }
}

TEST_F("require that regexp can be made from suffix string", ExprFixture()) {
    for (vespalib::string str: f1.expressions) {
        Regexp re(Regexp::make_from_suffix(str));
        EXPECT_TRUE(re.match(str));
        EXPECT_FALSE(re.match(str + "foo"));
        EXPECT_TRUE(re.match("foo" + str));
        EXPECT_FALSE(re.match("foo" + str + "bar"));
    }
}

TEST_F("require that regexp can be made from substring string", ExprFixture()) {
    for (vespalib::string str: f1.expressions) {
        Regexp re(Regexp::make_from_substring(str));
        EXPECT_TRUE(re.match(str));
        EXPECT_TRUE(re.match(str + "foo"));
        EXPECT_TRUE(re.match("foo" + str));
        EXPECT_TRUE(re.match("foo" + str + "bar"));
    }
}

TEST_MAIN() { TEST_RUN_ALL(); }
