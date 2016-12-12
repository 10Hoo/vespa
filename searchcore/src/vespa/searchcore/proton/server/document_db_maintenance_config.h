// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/vespalib/stllike/string.h>
#include <vespa/searchcore/proton/attribute/attribute_usage_filter_config.h>
#include <vespa/fastos/timestamp.h>

namespace proton {

class DocumentDBPruneConfig
{
private:
    double _interval;
    double _age;

public:
    DocumentDBPruneConfig(void);
    DocumentDBPruneConfig(double interval, double age);

    bool operator==(const DocumentDBPruneConfig &rhs) const;
    double getInterval(void) const { return _interval; }
    double getAge(void) const { return _age; }
};

typedef DocumentDBPruneConfig DocumentDBPruneRemovedDocumentsConfig;
typedef DocumentDBPruneConfig DocumentDBWipeOldRemovedFieldsConfig;

class DocumentDBHeartBeatConfig
{
private:
    double _interval;

public:
    DocumentDBHeartBeatConfig(void);
    DocumentDBHeartBeatConfig(double interval);

    bool operator==(const DocumentDBHeartBeatConfig &rhs) const;
    double getInterval(void) const { return _interval; }
};

class DocumentDBLidSpaceCompactionConfig
{
private:
    double   _interval;
    uint32_t _allowedLidBloat;
    double   _allowedLidBloatFactor;
    uint32_t _maxDocsToScan;

public:
    DocumentDBLidSpaceCompactionConfig();
    DocumentDBLidSpaceCompactionConfig(double interval,
                                       uint32_t allowedLidBloat,
                                       double allowwedLidBloatFactor,
                                       uint32_t maxDocsToScan = 10000);

    bool operator==(const DocumentDBLidSpaceCompactionConfig &rhs) const;
    double getInterval() const { return _interval; }
    uint32_t getAllowedLidBloat() const { return _allowedLidBloat; }
    double getAllowedLidBloatFactor() const { return _allowedLidBloatFactor; }
    uint32_t getMaxDocsToScan() const { return _maxDocsToScan; }
};

class DocumentDBMaintenanceConfig
{
public:
    typedef std::shared_ptr<DocumentDBMaintenanceConfig> SP;

private:
    DocumentDBPruneRemovedDocumentsConfig _pruneRemovedDocuments;
    DocumentDBHeartBeatConfig             _heartBeat;
    DocumentDBWipeOldRemovedFieldsConfig  _wipeOldRemovedFields;
    double                                _sessionCachePruneInterval;
    fastos::TimeStamp                     _visibilityDelay;
    DocumentDBLidSpaceCompactionConfig    _lidSpaceCompaction;
    AttributeUsageFilterConfig            _attributeUsageFilterConfig;
    double                                _attributeUsageSampleInterval;

public:
    DocumentDBMaintenanceConfig(void);

    DocumentDBMaintenanceConfig(const DocumentDBPruneRemovedDocumentsConfig &pruneRemovedDocuments,
                                const DocumentDBHeartBeatConfig &heartBeat,
                                const DocumentDBWipeOldRemovedFieldsConfig &wipeOldRemovedFields,
                                double sessionCachePruneInterval,
                                fastos::TimeStamp visibilityDelay,
                                const DocumentDBLidSpaceCompactionConfig &lidSpaceCompaction,
                                const AttributeUsageFilterConfig &attributeUsageFilterConfig,
                                double attributeUsageSampleInterval);

    bool
    operator==(const DocumentDBMaintenanceConfig &rhs) const;

    const DocumentDBPruneRemovedDocumentsConfig &
    getPruneRemovedDocumentsConfig(void) const
    {
        return _pruneRemovedDocuments;
    }

    const DocumentDBHeartBeatConfig &
    getHeartBeatConfig(void) const
    {
        return _heartBeat;
    }

    const DocumentDBWipeOldRemovedFieldsConfig &
    getWipeOldRemovedFieldsConfig() const
    {
        return _wipeOldRemovedFields;
    }

    double
    getSessionCachePruneInterval() const
    {
        return _sessionCachePruneInterval;
    }

    fastos::TimeStamp getVisibilityDelay() const { return _visibilityDelay; }

    const DocumentDBLidSpaceCompactionConfig &getLidSpaceCompactionConfig() const {
        return _lidSpaceCompaction;
    }

    const AttributeUsageFilterConfig &getAttributeUsageFilterConfig() const {
        return _attributeUsageFilterConfig;
    }

    double getAttributeUsageSampleInterval() const {
        return _attributeUsageSampleInterval;
    }
};

} // namespace proton

