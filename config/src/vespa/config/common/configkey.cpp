// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "configkey.h"

namespace config {

ConfigKey::ConfigKey(const vespalib::stringref & configId,
                     const vespalib::stringref & defName,
                     const vespalib::stringref & defNamespace,
                     const vespalib::stringref & defMd5)
    : _configId(configId),
      _defName(defName),
      _defNamespace(defNamespace),
      _defMd5(defMd5),
      _defSchema(),
      _key(_configId + _defName + _defNamespace)
{}

ConfigKey::ConfigKey(const vespalib::stringref & configId,
                     const vespalib::stringref & defName,
                     const vespalib::stringref & defNamespace,
                     const vespalib::stringref & defMd5,
                     const std::vector<vespalib::string> & defSchema)
    : _configId(configId),
      _defName(defName),
      _defNamespace(defNamespace),
      _defMd5(defMd5),
      _defSchema(defSchema),
      _key(_configId + _defName + _defNamespace)
{
}

ConfigKey::ConfigKey()
    : _configId(),
      _defName(),
      _defNamespace(),
      _defMd5(),
      _defSchema(),
      _key()
{}

ConfigKey::~ConfigKey() { }

bool
ConfigKey::operator<(const ConfigKey & rhs) const
{
    return _key < rhs._key;
}

bool
ConfigKey::operator>(const ConfigKey & rhs) const
{
    return _key > rhs._key;
}

bool
ConfigKey::operator==(const ConfigKey & rhs) const
{
    return _key.compare(rhs._key) == 0;
}

const vespalib::string & ConfigKey::getDefName() const { return _defName; }
const vespalib::string & ConfigKey::getConfigId() const { return _configId; }
const vespalib::string & ConfigKey::getDefNamespace() const { return _defNamespace; }
const vespalib::string & ConfigKey::getDefMd5() const { return _defMd5; }
const std::vector<vespalib::string> & ConfigKey::getDefSchema() const { return _defSchema; }

const vespalib::string
ConfigKey::toString() const
{
    vespalib::string s;
    s.append("name=");
    s.append(_defName);
    s.append(",namespace=");
    s.append(_defNamespace);
    s.append(",configId=");
    s.append(_configId);
    return s;
}

}
