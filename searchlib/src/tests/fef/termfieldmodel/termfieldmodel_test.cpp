// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/vespalib/testkit/test_kit.h>
#include <vespa/searchlib/fef/fef.h>
#include <vespa/searchlib/queryeval/searchiterator.h>

#include <algorithm>

using namespace search::fef;

struct State {
    SimpleTermData          term;
    MatchData::UP           md;
    TermFieldMatchData     *f3;
    TermFieldMatchData     *f5;
    TermFieldMatchData     *f7;
    TermFieldMatchDataArray array;

    State();
    ~State();

    void setArray(TermFieldMatchDataArray value) {
        array = value;
    }
};

State::State() : term(), md(), f3(0), f5(0), f7(0), array() {}
State::~State() {}

void testInvalidId() {
    const TermFieldMatchData empty;
    using search::queryeval::SearchIterator;

    EXPECT_EQUAL(TermFieldMatchData::invalidId(), empty.getDocId());
    EXPECT_TRUE(TermFieldMatchData::invalidId() < (SearchIterator::beginId() + 1 ) ||
               TermFieldMatchData::invalidId() > (search::endDocId - 1));
}

void testSetup(State &state) {
    MatchDataLayout layout;

    state.term.addField(3); // docfreq = 1
    state.term.addField(7); // docfreq = 2
    state.term.addField(5); // docfreq = 3

    typedef search::fef::ITermFieldRangeAdapter FRA;
    typedef search::fef::SimpleTermFieldRangeAdapter SFR;

    // lookup terms
    {
        int i = 1;
        for (SFR iter(state.term); iter.valid(); iter.next()) {
            iter.get().setDocFreq(0.25 * i++);
        }
    }

    // reserve handles
    {
        for (SFR iter(state.term); iter.valid(); iter.next()) {
            iter.get().setHandle(layout.allocTermField(iter.get().getFieldId()));
        }
    }

    state.md = layout.createMatchData();

    // init match data
    {
        for (FRA iter(state.term); iter.valid(); iter.next()) {
            const ITermFieldData& tfd = iter.get();

            TermFieldHandle handle = tfd.getHandle();
            TermFieldMatchData *data = state.md->resolveTermField(handle);
            switch (tfd.getFieldId()) {
            case 3:
                state.f3 = data;
                break;
            case 5:
                state.f5 = data;
                break;
            case 7:
                state.f7 = data;
                break;
            default:
                EXPECT_TRUE(false);
            }
        }
        EXPECT_EQUAL(3u, state.f3->getFieldId());
        EXPECT_EQUAL(5u, state.f5->getFieldId());
        EXPECT_EQUAL(7u, state.f7->getFieldId());
    }

    // test that we can setup array
    EXPECT_EQUAL(false, state.array.valid());
    state.setArray(TermFieldMatchDataArray().add(state.f3).add(state.f5).add(state.f7));
    EXPECT_EQUAL(true, state.array.valid());
}

void testGenerate(State &state) {
    // verify array
    EXPECT_EQUAL(3u, state.array.size());
    EXPECT_EQUAL(state.f3, state.array[0]);
    EXPECT_EQUAL(state.f5, state.array[1]);
    EXPECT_EQUAL(state.f7, state.array[2]);

    // stale unpacked data
    state.f5->reset(5);
    EXPECT_EQUAL(5u, state.f5->getDocId());
    {
        TermFieldMatchDataPosition pos;
        pos.setPosition(3);
        pos.setElementId(0);
        pos.setElementLen(10);
        state.f5->appendPosition(pos);
        EXPECT_EQUAL(1u, state.f5->getIterator().size());
        EXPECT_EQUAL(10u, state.f5->getIterator().getFieldLength());
    }
    state.f5->reset(6);
    EXPECT_EQUAL(6u, state.f5->getDocId());
    EXPECT_EQUAL(FieldPositionsIterator::UNKNOWN_LENGTH,
               state.f5->getIterator().getFieldLength());
    EXPECT_EQUAL(0u, state.f5->getIterator().size());


    // fresh unpacked data
    state.f3->reset(10);
    {
        TermFieldMatchDataPosition pos;
        pos.setPosition(3);
        pos.setElementId(0);
        pos.setElementLen(10);
        EXPECT_EQUAL(FieldPositionsIterator::UNKNOWN_LENGTH,
                   state.f3->getIterator().getFieldLength());
        state.f3->appendPosition(pos);
        EXPECT_EQUAL(10u, state.f3->getIterator().getFieldLength());
    }
    {
        TermFieldMatchDataPosition pos;
        pos.setPosition(15);
        pos.setElementId(1);
        pos.setElementLen(20);
        state.f3->appendPosition(pos);
        EXPECT_EQUAL(20u, state.f3->getIterator().getFieldLength());
    }
    {
        TermFieldMatchDataPosition pos;
        pos.setPosition(1);
        pos.setElementId(2);
        pos.setElementLen(5);
        state.f3->appendPosition(pos);
        EXPECT_EQUAL(20u, state.f3->getIterator().getFieldLength());
    }

    // raw score
    state.f7->setRawScore(10, 5.0);
}

void testAnalyze(State &state) {
    EXPECT_EQUAL(10u, state.f3->getDocId());
    EXPECT_NOT_EQUAL(10u, state.f5->getDocId());
    EXPECT_EQUAL(10u, state.f7->getDocId());

    FieldPositionsIterator it = state.f3->getIterator();
    EXPECT_EQUAL(20u, it.getFieldLength());
    EXPECT_EQUAL(3u, it.size());
    EXPECT_TRUE(it.valid());
    EXPECT_EQUAL(3u, it.getPosition());
    EXPECT_EQUAL(0u, it.getElementId());
    EXPECT_EQUAL(10u, it.getElementLen());
    it.next();
    EXPECT_TRUE(it.valid());
    EXPECT_EQUAL(15u, it.getPosition());
    EXPECT_EQUAL(1u, it.getElementId());
    EXPECT_EQUAL(20u, it.getElementLen());
    it.next();
    EXPECT_TRUE(it.valid());
    EXPECT_EQUAL(1u, it.getPosition());
    EXPECT_EQUAL(2u, it.getElementId());
    EXPECT_EQUAL(5u, it.getElementLen());
    it.next();
    EXPECT_TRUE(!it.valid());

    EXPECT_EQUAL(0.0, state.f3->getRawScore());
    EXPECT_EQUAL(0.0, state.f5->getRawScore());
    EXPECT_EQUAL(5.0, state.f7->getRawScore());
}

TEST("term field model") {
    State state;
    testSetup(state);
    testGenerate(state);
    testAnalyze(state);
    testInvalidId();
}

TEST("Access subqueries") {
    State state;
    testSetup(state);
    state.f3->reset(10);
    state.f3->setSubqueries(10, 42);
    EXPECT_EQUAL(42ULL, state.f3->getSubqueries());
    state.f3->enableRawScore();
    EXPECT_EQUAL(0ULL, state.f3->getSubqueries());

    state.f3->reset(11);
    state.f3->appendPosition(TermFieldMatchDataPosition());
    state.f3->setSubqueries(11, 42);
    EXPECT_EQUAL(0ULL, state.f3->getSubqueries());
}

TEST("require that TermFieldMatchData can be tagged as needed or not") {
    TermFieldMatchData tfmd;
    tfmd.setFieldId(123);
    EXPECT_EQUAL(tfmd.getFieldId(),123u);
    EXPECT_TRUE(!tfmd.isNotNeeded());
    tfmd.tagAsNotNeeded();
    EXPECT_EQUAL(tfmd.getFieldId(),123u);
    EXPECT_TRUE(tfmd.isNotNeeded());
    tfmd.tagAsNeeded();
    EXPECT_EQUAL(tfmd.getFieldId(),123u);
    EXPECT_TRUE(!tfmd.isNotNeeded());
}

TEST_MAIN() { TEST_RUN_ALL(); }
