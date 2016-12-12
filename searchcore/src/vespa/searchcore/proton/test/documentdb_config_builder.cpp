// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "documentdb_config_builder.h"
#include <vespa/config-summary.h>
#include <vespa/config-summarymap.h>
#include <vespa/config-rank-profiles.h>
#include <vespa/config-attributes.h>
#include <vespa/config-indexschema.h>
#include <vespa/searchsummary/config/config-juniperrc.h>
#include <vespa/document/config/config-documenttypes.h>

using document::DocumenttypesConfig;
using search::TuneFileDocumentDB;
using search::index::Schema;
using vespa::config::search::RankProfilesConfig;
using vespa::config::search::IndexschemaConfig;
using vespa::config::search::AttributesConfig;
using vespa::config::search::SummaryConfig;
using vespa::config::search::SummarymapConfig;
using vespa::config::search::summary::JuniperrcConfig;

namespace proton {
namespace test {

DocumentDBConfigBuilder::DocumentDBConfigBuilder(int64_t generation,
                                                 const search::index::Schema::SP &schema,
                                                 const vespalib::string &configId,
                                                 const vespalib::string &docTypeName)
    : _generation(generation),
      _rankProfiles(std::make_shared<RankProfilesConfig>()),
      _rankingConstants(std::make_shared<matching::RankingConstants>()),
      _indexschema(std::make_shared<IndexschemaConfig>()),
      _attributes(std::make_shared<AttributesConfig>()),
      _summary(std::make_shared<SummaryConfig>()),
      _summarymap(std::make_shared<SummarymapConfig>()),
      _juniperrc(std::make_shared<JuniperrcConfig>()),
      _documenttypes(std::make_shared<DocumenttypesConfig>()),
      _repo(std::make_shared<document::DocumentTypeRepo>()),
      _tuneFileDocumentDB(std::make_shared<TuneFileDocumentDB>()),
      _schema(schema),
      _maintenance(std::make_shared<DocumentDBMaintenanceConfig>()),
      _configId(configId),
      _docTypeName(docTypeName),
      _extraConfig()
{ }

DocumentDBConfig::SP
DocumentDBConfigBuilder::build()
{
    return std::make_shared<DocumentDBConfig>(
            _generation,
            _rankProfiles,
            _rankingConstants,
            _indexschema,
            _attributes,
            _summary,
            _summarymap,
            _juniperrc,
            _documenttypes,
            _repo,
            _tuneFileDocumentDB,
            _schema,
            _maintenance,
            _configId,
            _docTypeName,
            _extraConfig);
}

}
}
