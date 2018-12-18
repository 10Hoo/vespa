// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "configconverter.h"

using namespace vespa::config::search;
using namespace search;


namespace {

using search::attribute::CollectionType;
using search::attribute::BasicType;
using vespalib::eval::ValueType;

typedef std::map<AttributesConfig::Attribute::Datatype, BasicType::Type> DataTypeMap;
typedef std::map<AttributesConfig::Attribute::Collectiontype, CollectionType::Type> CollectionTypeMap;

DataTypeMap
getDataTypeMap()
{
    DataTypeMap map;
    map[AttributesConfig::Attribute::STRING] = BasicType::STRING;
    map[AttributesConfig::Attribute::BOOL] = BasicType::BOOL;
    map[AttributesConfig::Attribute::UINT2] = BasicType::UINT2;
    map[AttributesConfig::Attribute::UINT4] = BasicType::UINT4;
    map[AttributesConfig::Attribute::INT8] = BasicType::INT8;
    map[AttributesConfig::Attribute::INT16] = BasicType::INT16;
    map[AttributesConfig::Attribute::INT32] = BasicType::INT32;
    map[AttributesConfig::Attribute::INT64] = BasicType::INT64;
    map[AttributesConfig::Attribute::FLOAT] = BasicType::FLOAT;
    map[AttributesConfig::Attribute::DOUBLE] = BasicType::DOUBLE;
    map[AttributesConfig::Attribute::PREDICATE] = BasicType::PREDICATE;
    map[AttributesConfig::Attribute::TENSOR] = BasicType::TENSOR;
    map[AttributesConfig::Attribute::REFERENCE] = BasicType::REFERENCE;
    map[AttributesConfig::Attribute::NONE] = BasicType::NONE;
    return map;
}

CollectionTypeMap
getCollectionTypeMap()
{
    CollectionTypeMap map;
    map[AttributesConfig::Attribute::SINGLE] = CollectionType::SINGLE;
    map[AttributesConfig::Attribute::ARRAY] = CollectionType::ARRAY;
    map[AttributesConfig::Attribute::WEIGHTEDSET] = CollectionType::WSET;
    return map;
}

static DataTypeMap _dataTypeMap = getDataTypeMap();
static CollectionTypeMap _collectionTypeMap = getCollectionTypeMap();

}

namespace search::attribute {

Config
ConfigConverter::convert(const AttributesConfig::Attribute & cfg)
{
    BasicType bType(_dataTypeMap[cfg.datatype]);
    CollectionType cType(_collectionTypeMap[cfg.collectiontype]);
    cType.removeIfZero(cfg.removeifzero);
    cType.createIfNonExistant(cfg.createifnonexistent);
    Config retval(bType, cType);
    PredicateParams predicateParams;
    retval.setFastSearch(cfg.fastsearch);
    retval.setHuge(cfg.huge);
    retval.setEnableBitVectors(cfg.enablebitvectors);
    retval.setEnableOnlyBitVector(cfg.enableonlybitvector);
    retval.setIsFilter(cfg.enableonlybitvector);
    retval.setFastAccess(cfg.fastaccess);
    retval.setMutable(cfg.ismutable);
    predicateParams.setArity(cfg.arity);
    predicateParams.setBounds(cfg.lowerbound, cfg.upperbound);
    predicateParams.setDensePostingListThreshold(cfg.densepostinglistthreshold);
    retval.setPredicateParams(predicateParams);
    if (retval.basicType().type() == BasicType::Type::TENSOR) {
        if (!cfg.tensortype.empty()) {
            retval.setTensorType(ValueType::from_spec(cfg.tensortype));
        } else {
            retval.setTensorType(ValueType::tensor_type({}));
        }
    }
    return retval;
}

}
