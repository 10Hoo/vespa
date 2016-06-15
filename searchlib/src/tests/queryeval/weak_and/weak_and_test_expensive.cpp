// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include "wand_bench_setup.hpp"

using namespace rise;

namespace {

template <typename WeakAndType, typename RiseType>
void checkWandHits(WandFactory &vespa, WandFactory &rise, uint32_t step, uint32_t filter) {
    WandSetup vespaSetup(vespa, 500, 5000000);
    WandSetup riseSetup(rise, 500, 5000000);
    SearchIterator::UP s1 = vespaSetup.create();
    s1->initFullRange();
    SearchIterator::UP s2 = riseSetup.create();
    s2->initFullRange();
    ASSERT_TRUE(dynamic_cast<WeakAndType*>(s1.get()) != 0);
    ASSERT_TRUE(dynamic_cast<WeakAndType*>(s2.get()) == 0);
    ASSERT_TRUE(dynamic_cast<RiseType*>(s2.get()) != 0);
    ASSERT_TRUE(dynamic_cast<RiseType*>(s1.get()) == 0);
    s1->seek(1);
    s2->seek(1);
    while (!s1->isAtEnd() &&
           !s2->isAtEnd())
    {
        ASSERT_EQUAL(s1->getDocId(), s2->getDocId());
        if ((filter == 0) || ((s1->getDocId() % filter) != 0)) {
            s1->unpack(s1->getDocId());
            s2->unpack(s2->getDocId());
        }
        s1->seek(s1->getDocId() + step);
        s2->seek(s2->getDocId() + step);
    }
    ASSERT_TRUE(s1->isAtEnd());
    ASSERT_TRUE(s2->isAtEnd());
}

} // namespace <unnamed>

TEST("require that mod search works") {
    Stats stats;
    SearchIterator::UP search(new ModSearch(stats, 3, 8, 3, NULL));
    SimpleResult hits;
    hits.search(*search);
    EXPECT_EQUAL(SimpleResult().addHit(3).addHit(6), hits);
}

//---- WeakAndSearch ------------------------------------------------------------------------------

TEST_FF("require that (array) WAND and RISE WAND gives the same hits",
        VespaArrayWandFactory(500), TermFrequencyRiseWandFactory(500))
{
    checkWandHits<WeakAndSearch, TermFrequencyRiseWand>(f1, f2, 1, 0);
}

TEST_FF("require that (heap) WAND and RISE WAND gives the same hits",
        VespaHeapWandFactory(500), TermFrequencyRiseWandFactory(500))
{
    checkWandHits<WeakAndSearch, TermFrequencyRiseWand>(f1, f2, 1, 0);
}

TEST_FF("require that (array) WAND and RISE WAND gives the same hits with filtering and skipping",
        VespaArrayWandFactory(500), TermFrequencyRiseWandFactory(500))
{
    checkWandHits<WeakAndSearch, TermFrequencyRiseWand>(f1, f2, 123, 5);
}

TEST_FF("require that (heap) WAND and RISE WAND gives the same hits with filtering and skipping",
        VespaHeapWandFactory(500), TermFrequencyRiseWandFactory(500))
{
    checkWandHits<WeakAndSearch, TermFrequencyRiseWand>(f1, f2, 123, 5);
}


//---- ParallelWeakAndSearch ----------------------------------------------------------------------

TEST_FF("require that (array) PWAND and RISE WAND gives the same hits",
        VespaParallelArrayWandFactory(500), DotProductRiseWandFactory(500))
{
    checkWandHits<ParallelWeakAndSearch, DotProductRiseWand>(f1, f2, 1, 0);
}

TEST_FF("require that (heap) PWAND and RISE WAND gives the same hits",
        VespaParallelHeapWandFactory(500), DotProductRiseWandFactory(500))
{
    checkWandHits<ParallelWeakAndSearch, DotProductRiseWand>(f1, f2, 1, 0);
}

TEST_FF("require that (array) PWAND and RISE WAND gives the same hits with filtering and skipping",
        VespaParallelArrayWandFactory(500), DotProductRiseWandFactory(500))
{
    checkWandHits<ParallelWeakAndSearch, DotProductRiseWand>(f1, f2, 123, 5);
}

TEST_FF("require that (heap) PWAND and RISE WAND gives the same hits with filtering and skipping",
        VespaParallelHeapWandFactory(500), DotProductRiseWandFactory(500))
{
    checkWandHits<ParallelWeakAndSearch, DotProductRiseWand>(f1, f2, 123, 5);
}


TEST_MAIN() { TEST_RUN_ALL(); }
