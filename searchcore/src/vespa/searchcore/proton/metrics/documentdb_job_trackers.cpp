// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".proton.metrics.documentdb_job_trackers");
#include "documentdb_job_trackers.h"
#include "job_tracked_flush_target.h"

using searchcorespi::IFlushTarget;
typedef IFlushTarget::Type FTT;
typedef IFlushTarget::Component FTC;

namespace proton {

DocumentDBJobTrackers::DocumentDBJobTrackers()
    : _lock(),
      _now(fastos::ClockSystem::now()),
      _attributeFlush(new JobTracker(_now.sec(), _lock)),
      _memoryIndexFlush(new JobTracker(_now.sec(), _lock)),
      _diskIndexFusion(new JobTracker(_now.sec(), _lock)),
      _documentStoreFlush(new JobTracker(_now.sec(), _lock)),
      _documentStoreCompact(new JobTracker(_now.sec(), _lock)),
      _bucketMove(new JobTracker(_now.sec(), _lock)),
      _lidSpaceCompact(new JobTracker(_now.sec(), _lock)),
      _removedDocumentsPrune(new JobTracker(_now.sec(), _lock))
{
}

namespace {

IFlushTarget::SP
trackFlushTarget(const IJobTracker::SP &tracker,
                 const IFlushTarget::SP &target)
{
    return IFlushTarget::SP(new JobTrackedFlushTarget(tracker, target));
}

}

IFlushTarget::List
DocumentDBJobTrackers::trackFlushTargets(const IFlushTarget::List &flushTargets)
{
    IFlushTarget::List retval;
    for (const auto &ft : flushTargets) {
        if (ft->getComponent() == FTC::ATTRIBUTE && ft->getType() == FTT::SYNC) {
            retval.push_back(trackFlushTarget(_attributeFlush, ft));
        } else if (ft->getComponent() == FTC::INDEX && ft->getType() == FTT::FLUSH) {
            retval.push_back(trackFlushTarget(_memoryIndexFlush, ft));
        } else if (ft->getComponent() == FTC::INDEX && ft->getType() == FTT::GC) {
            retval.push_back(trackFlushTarget(_diskIndexFusion, ft));
        } else if (ft->getComponent() == FTC::DOCUMENT_STORE && ft->getType() == FTT::SYNC) {
            retval.push_back(trackFlushTarget(_documentStoreFlush, ft));
        } else if (ft->getComponent() == FTC::DOCUMENT_STORE && ft->getType() == FTT::GC) {
            retval.push_back(trackFlushTarget(_documentStoreCompact, ft));
        } else {
            LOG(warning, "trackFlushTargets(): Flush target '%s' with type '%d' and component '%d' "
                    "is not known and will not be tracked",
                    ft->getName().c_str(), static_cast<int>(ft->getType()),
                    static_cast<int>(ft->getComponent()));
            retval.push_back(ft);
        }
    }
    return retval;
}

namespace {

double
updateMetric(metrics::DoubleAverageMetric &metric,
             JobTracker &tracker,
             double nowInSec,
             const vespalib::LockGuard &guard)
{
    double load = tracker.sampleLoad(nowInSec, guard);
    metric.addValue(load);
    return load;
}

}

void
DocumentDBJobTrackers::updateMetrics(DocumentDBTaggedMetrics::JobMetrics &metrics)
{
    vespalib::LockGuard guard(_lock);
    _now = fastos::ClockSystem::now();
    double nowInSec = _now.sec();
    double load = 0.0;
    load += updateMetric(metrics.attributeFlush, *_attributeFlush, nowInSec, guard);
    load += updateMetric(metrics.memoryIndexFlush, *_memoryIndexFlush, nowInSec, guard);
    load += updateMetric(metrics.diskIndexFusion, *_diskIndexFusion, nowInSec, guard);
    load += updateMetric(metrics.documentStoreFlush, *_documentStoreFlush, nowInSec, guard);
    load += updateMetric(metrics.documentStoreCompact, *_documentStoreCompact, nowInSec, guard);
    load += updateMetric(metrics.bucketMove, *_bucketMove, nowInSec, guard);
    load += updateMetric(metrics.lidSpaceCompact, *_lidSpaceCompact, nowInSec, guard);
    load += updateMetric(metrics.removedDocumentsPrune, *_removedDocumentsPrune, nowInSec, guard);
    metrics.total.addValue(load);
}

} // namespace proton
