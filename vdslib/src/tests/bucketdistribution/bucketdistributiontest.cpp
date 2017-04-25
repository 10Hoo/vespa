// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <cppunit/extensions/HelperMacros.h>
#include <vespa/vdslib/bucketdistribution.h>

using namespace vdslib;

class BucketDistributionTest : public CppUnit::TestFixture {
public:
    void testDistribution();
    void testNumBucketBits();

public:
    CPPUNIT_TEST_SUITE(BucketDistributionTest);
    CPPUNIT_TEST(testDistribution);
    CPPUNIT_TEST(testNumBucketBits);
    CPPUNIT_TEST_SUITE_END();

private:
    void assertDistribution(uint32_t numColumns, uint32_t numBucketBits, const uint32_t expected[]);
};

CPPUNIT_TEST_SUITE_REGISTRATION(BucketDistributionTest);

void BucketDistributionTest::testDistribution()
{
    const uint32_t expected4[] = {
        10, 11, 9, 6, 4, 8, 14, 1, 13, 2, 12, 3, 5, 7, 15, 0 };
    assertDistribution(16, 4, expected4);

    const uint32_t expected5[] = {
        11, 12, 11, 13, 8, 13, 8, 9, 4, 5, 6, 12, 10, 15, 1, 1, 7, 9, 14, 2, 2, 14, 3, 3, 4, 5, 6, 7, 10, 15, 0, 0 };
    assertDistribution(16, 5, expected5);

    const uint32_t expected6[] = {
        13, 13, 13, 13, 9, 11, 8, 9, 11, 14, 9, 11, 14, 14, 8, 10, 11, 14, 4, 5, 5, 6, 6, 7, 8, 10, 12, 15, 1, 1, 1, 1,
        6, 7, 8, 10, 12, 15, 2, 2, 2, 2, 12, 15, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6, 7, 7, 9, 10, 12, 15, 0, 0, 0, 0 };
    assertDistribution(16, 6, expected6);

    const uint32_t expected7[] = {
        14, 14, 14, 13, 11, 14, 14, 10, 13, 14, 10, 12, 8, 8, 9, 10, 9, 10, 11, 12, 13, 15, 11, 12, 13, 13, 15, 8, 8, 9, 10, 11,
        11, 12, 13, 15, 4, 4, 5, 5, 5, 5, 15, 6, 6, 7, 7, 7, 8, 9, 9, 10, 11, 12, 14, 15, 1, 1, 1, 1, 1, 1, 1, 1,
        6, 6, 6, 7, 7, 8, 8, 9, 10, 11, 12, 13, 15, 2, 2, 2, 2, 2, 2, 2, 2, 12, 13, 15, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 9, 9, 10, 11, 12, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0 };
    assertDistribution(16, 7, expected7);

    const uint32_t expected8[] = {
        15, 14, 15, 15, 12, 14, 15, 12, 12, 11, 12, 13, 14, 13, 13, 13, 14, 15, 15, 9, 14, 9, 15, 10, 11, 11, 12, 8, 8, 8, 8, 9,
        9, 10, 10, 10, 11, 11, 12, 13, 13, 14, 10, 11, 11, 12, 13, 13, 14, 13, 13, 14, 15, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11,
        10, 10, 11, 11, 12, 13, 13, 14, 15, 4, 4, 4, 4, 15, 5, 5, 5, 5, 5, 5, 5, 15, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
        8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12, 13, 14, 14, 15, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12, 13, 14, 14, 15, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 12, 12, 13, 14, 14, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
        8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12, 13, 14, 15, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    assertDistribution(16, 8, expected8);

    const uint32_t expected9[] = {
        15, 15, 15, 15, 14, 14, 15, 13, 13, 13, 13, 13, 14, 14, 15, 12, 14, 15, 15, 11, 11, 12, 13, 13, 13, 14, 14, 15, 15, 10, 12, 12,
        12, 13, 13, 13, 14, 14, 15, 15, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 14, 15, 15, 8, 8, 8, 8, 9, 9, 9,
        9, 9, 9, 10, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 14, 14, 14, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12,
        13, 13, 14, 14, 14, 15, 15, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11,
        10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 4, 4, 4, 4, 4, 4, 4, 15, 15, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 14, 14, 14, 15, 15, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9,
        9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    assertDistribution(16, 9, expected9);
}

void
BucketDistributionTest::assertDistribution(uint32_t numColumns, uint32_t numBucketBits, const uint32_t expected[])
{
    BucketDistribution bd(numColumns, numBucketBits);
    CPPUNIT_ASSERT_EQUAL(numColumns, bd.getNumColumns());
    CPPUNIT_ASSERT_EQUAL((uint32_t)(1 << numBucketBits), bd.getNumBuckets());
    for (uint32_t i = 0; i < bd.getNumBuckets(); ++i) {
        CPPUNIT_ASSERT_EQUAL(expected[i],
                             bd.getColumn(document::BucketId(16, i)));
    }
}

void BucketDistributionTest::testNumBucketBits()
{
    BucketDistribution bd(1, 4);
    for (uint32_t i = 0; i <= 0xf; ++i) {
        CPPUNIT_ASSERT_EQUAL(0u, bd.getColumn(document::BucketId(32, (rand() << 4) & i)));
    }

    bd.reset();
    bd.setNumColumns(1);
    bd.setNumBucketBits(8);
    for (uint32_t i = 0; i <= 0xff; ++i) {
        CPPUNIT_ASSERT_EQUAL(0u, bd.getColumn(document::BucketId(32, (rand() << 8) & i)));
    }

    bd.reset();
    bd.setNumColumns(1);
    bd.setNumBucketBits(16);
    for (uint32_t i = 0; i <= 0xffff; ++i) {
        CPPUNIT_ASSERT_EQUAL(0u, bd.getColumn(document::BucketId(32, (rand() << 16) & i)));
    }
}
