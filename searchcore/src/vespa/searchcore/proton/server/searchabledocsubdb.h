// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "documentdbconfig.h"
#include "searchable_doc_subdb_configurer.h"
#include "executorthreadingservice.h"
#include "fast_access_doc_subdb.h"
#include "feedhandler.h"
#include "searchable_feed_view.h"
#include "searchview.h"
#include "summaryadapter.h"
#include <memory>
#include <vector>
#include <vespa/searchcore/proton/attribute/attributemanager.h>
#include <vespa/searchcore/proton/common/doctypename.h>
#include <vespa/searchcore/proton/docsummary/summarymanager.h>
#include <vespa/searchcore/proton/documentmetastore/documentmetastorecontext.h>
#include <vespa/searchcorespi/index/iindexmanager.h>
#include <vespa/searchcore/proton/index/i_index_writer.h>
#include <vespa/searchcore/proton/index/indexmanager.h>
#include <vespa/searchcore/proton/matching/constant_value_repo.h>
#include <vespa/searchcore/config/config-proton.h>
#include <vespa/vespalib/eval/value_cache/constant_tensor_loader.h>
#include <vespa/vespalib/eval/value_cache/constant_value_cache.h>
#include <vespa/vespalib/util/blockingthreadstackexecutor.h>
#include <vespa/vespalib/util/varholder.h>


namespace proton
{

class MetricsWireService;
class DocumentDBMetrics;

/**
 * The searchable sub database supports searching and keeps all attribute fields in memory and
 * inserts all index fields into the memory index in addition to storing documents in the
 * underlying document store.
 *
 * This class is used directly by the "0.ready" sub database for handling all ready documents.
 */
class SearchableDocSubDB : public FastAccessDocSubDB,
                           public searchcorespi::IIndexManager::Reconfigurer

{
public:
    struct Config {
        const FastAccessDocSubDB::Config _fastUpdCfg;
        const size_t _numSearcherThreads;

        Config(const FastAccessDocSubDB::Config &fastUpdCfg,
               size_t numSearcherThreads)
            : _fastUpdCfg(fastUpdCfg),
              _numSearcherThreads(numSearcherThreads)
        {
        }
    };

    struct Context {
        const FastAccessDocSubDB::Context _fastUpdCtx;
        matching::QueryLimiter   &_queryLimiter;
        const vespalib::Clock    &_clock;
        vespalib::ThreadExecutor &_warmupExecutor;

        Context(const FastAccessDocSubDB::Context &fastUpdCtx,
                matching::QueryLimiter &queryLimiter,
                const vespalib::Clock &clock,
                vespalib::ThreadExecutor &warmupExecutor)
            : _fastUpdCtx(fastUpdCtx),
              _queryLimiter(queryLimiter),
              _clock(clock),
              _warmupExecutor(warmupExecutor)
        {
        }
    };

private:
    typedef FastAccessDocSubDB Parent;

    IIndexManager::SP                           _indexMgr;
    IIndexWriter::SP                            _indexWriter;
    vespalib::VarHolder<SearchView::SP>         _rSearchView;
    vespalib::VarHolder<SearchableFeedView::SP> _rFeedView;
    vespalib::eval::ConstantTensorLoader        _tensorLoader;
    vespalib::eval::ConstantValueCache          _constantValueCache;
    matching::ConstantValueRepo                 _constantValueRepo;
    SearchableDocSubDBConfigurer                _configurer;
    const size_t                                _numSearcherThreads;
    vespalib::ThreadExecutor                   &_warmupExecutor;

    // Note: lifetime of indexManager must be handled by caller.
    initializer::InitializerTask::SP
    createIndexManagerInitializer(const DocumentDBConfig &configSnapshot,
                                  const search::index::Schema::SP &unionSchema,
                                  const vespa::config::search::core::ProtonConfig::Index &indexCfg,
                                  std::shared_ptr<searchcorespi::IIndexManager::SP> indexManager) const;

    void setupIndexManager(searchcorespi::IIndexManager::SP indexManager);

    void
    initFeedView(const IAttributeWriter::SP &attrWriter,
                 const DocumentDBConfig &configSnapshot);

    void
    reconfigureMatchingMetrics(const vespa::config::search::RankProfilesConfig &config);

    /**
     * Implements IndexManagerReconfigurer API.
     */
    virtual bool
    reconfigure(vespalib::Closure0<bool>::UP closure);

    void
    reconfigureIndexSearchable();

    void
    syncViews();

protected:
    virtual IFlushTarget::List
    getFlushTargetsInternal();

    using Parent::updateLidReuseDelayer;

    virtual void
    updateLidReuseDelayer(const LidReuseDelayerConfig &config) override;
public:
    SearchableDocSubDB(const Config &cfg,
                       const Context &ctx);

    virtual
    ~SearchableDocSubDB();

    virtual DocumentSubDbInitializer::UP
    createInitializer(const DocumentDBConfig &configSnapshot,
                      SerialNum configSerialNum,
                      const search::index::Schema::SP &unionSchema,
                      const vespa::config::search::core::
                      ProtonConfig::Summary &protonSummaryCfg,
                      const vespa::config::search::core::
                      ProtonConfig::Index &indexCfg) const override;

    virtual void setup(const DocumentSubDbInitializerResult &initResult)
        override;

    virtual void
    initViews(const DocumentDBConfig &configSnapshot,
              const matching::SessionManager::SP &sessionManager);

    virtual IReprocessingTask::List
    applyConfig(const DocumentDBConfig &newConfigSnapshot,
                const DocumentDBConfig &oldConfigSnapshot,
                SerialNum serialNum,
                const ReconfigParams params);

    virtual void
    clearViews()
    {
        _rFeedView.clear();
        _rSearchView.clear();
        Parent::clearViews();
    }

    virtual proton::IAttributeManager::SP
    getAttributeManager() const
    {
        return _rSearchView.get()->getAttributeManager();
    }

    virtual const IIndexManager::SP &
    getIndexManager() const
    {
        return _indexMgr;
    }

    virtual const IIndexWriter::SP &
    getIndexWriter() const
    {
        return _indexWriter;
    }

    virtual SerialNum
    getOldestFlushedSerial();

    virtual SerialNum
    getNewestFlushedSerial();

    virtual void
    wipeHistory(SerialNum wipeSerial,
                const search::index::Schema &newHistorySchema,
                const search::index::Schema &wipeSchema);

    virtual void
    setIndexSchema(const search::index::Schema::SP &schema,
                   const search::index::Schema::SP &fusionSchema);

    virtual size_t
    getNumActiveDocs() const override;

    virtual search::SearchableStats
    getSearchableStats() const;

    virtual IDocumentRetriever::UP
    getDocumentRetriever();

    virtual matching::MatchingStats
    getMatcherStats(const vespalib::string &rankProfile) const;
};

} // namespace proton

