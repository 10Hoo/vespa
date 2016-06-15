// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/vespalib/testkit/test_kit.h>
#include <vespa/searchcommon/attribute/config.h>

using search::attribute::Config;
using search::attribute::BasicType;
using search::attribute::CollectionType;
using vespalib::tensor::TensorType;


struct Fixture
{
    Config _config;
    Fixture()
        : _config()
    {
    }

    Fixture(BasicType bt,
            CollectionType ct = CollectionType::SINGLE,
            bool fastSearch_ = false,
            bool huge_ = false)
        : _config(bt, ct, fastSearch_, huge_)
    {
    }
};

TEST_F("test default attribute config", Fixture)
{
    EXPECT_EQUAL(BasicType::Type::NONE, f._config.basicType().type());
    EXPECT_EQUAL(CollectionType::Type::SINGLE,
                 f._config.collectionType().type());
    EXPECT_TRUE(!f._config.fastSearch());
    EXPECT_TRUE(!f._config.huge());
    EXPECT_TRUE(!f._config.getEnableBitVectors());
    EXPECT_TRUE(!f._config.getEnableOnlyBitVector());
    EXPECT_TRUE(!f._config.getIsFilter());
    EXPECT_TRUE(!f._config.fastAccess());
    EXPECT_TRUE(!f._config.tensorType().is_valid());
}

TEST_F("test integer weightedset attribute config",
       Fixture(BasicType::Type::INT32,
               CollectionType::Type::WSET))
{
    EXPECT_EQUAL(BasicType::Type::INT32, f._config.basicType().type());
    EXPECT_EQUAL(CollectionType::Type::WSET,
                 f._config.collectionType().type());
    EXPECT_TRUE(!f._config.fastSearch());
    EXPECT_TRUE(!f._config.huge());
    EXPECT_TRUE(!f._config.getEnableBitVectors());
    EXPECT_TRUE(!f._config.getEnableOnlyBitVector());
    EXPECT_TRUE(!f._config.getIsFilter());
    EXPECT_TRUE(!f._config.fastAccess());
    EXPECT_TRUE(!f._config.tensorType().is_valid());
}


TEST("test operator== on attribute config")
{
    Config cfg1(BasicType::Type::INT32, CollectionType::Type::WSET);
    Config cfg2(BasicType::Type::INT32, CollectionType::Type::ARRAY);
    Config cfg3(BasicType::Type::INT32, CollectionType::Type::WSET);

    EXPECT_TRUE(cfg1 != cfg2);
    EXPECT_TRUE(cfg2 != cfg3);
    EXPECT_TRUE(cfg1 == cfg3);
}


TEST("test operator== on attribute config for tensor type")
{
    Config cfg1(BasicType::Type::TENSOR);
    Config cfg2(BasicType::Type::TENSOR);
    Config cfg3(BasicType::Type::TENSOR);

    TensorType dense_x = TensorType::fromSpec("tensor(x[10])");
    TensorType sparse_x = TensorType::fromSpec("tensor(x{})");

    // invalid tensors are not equal
    EXPECT_TRUE(cfg1 != cfg2);
    EXPECT_TRUE(cfg2 != cfg3);
    EXPECT_TRUE(cfg1 != cfg3);

    cfg1.setTensorType(dense_x);
    cfg3.setTensorType(dense_x);
    EXPECT_EQUAL(dense_x, cfg1.tensorType());
    EXPECT_EQUAL(dense_x, cfg3.tensorType());
    EXPECT_TRUE(cfg1.tensorType().is_valid());
    EXPECT_TRUE(!cfg2.tensorType().is_valid());
    EXPECT_TRUE(cfg3.tensorType().is_valid());

    EXPECT_TRUE(cfg1 != cfg2);
    EXPECT_TRUE(cfg2 != cfg3);
    EXPECT_TRUE(cfg1 == cfg3);

    cfg3.setTensorType(sparse_x);
    EXPECT_EQUAL(sparse_x, cfg3.tensorType());
    EXPECT_TRUE(cfg3.tensorType().is_valid());
    EXPECT_TRUE(cfg1 != cfg3);
}


TEST_MAIN() { TEST_RUN_ALL(); }
