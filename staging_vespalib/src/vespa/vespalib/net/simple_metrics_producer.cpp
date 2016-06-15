// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "simple_metrics_producer.h"

namespace vespalib {

SimpleMetricsProducer::SimpleMetricsProducer()
    : _lock(),
      _metrics(),
      _totalMetrics()
{
}

void
SimpleMetricsProducer::setMetrics(const vespalib::string &metrics)
{
    LockGuard guard(_lock);
    _metrics = metrics;
}

vespalib::string
SimpleMetricsProducer::getMetrics(const vespalib::string &)
{
    LockGuard guard(_lock);
    return _metrics;
}

void
SimpleMetricsProducer::setTotalMetrics(const vespalib::string &metrics)
{
    LockGuard guard(_lock);
    _totalMetrics = metrics;
}

vespalib::string
SimpleMetricsProducer::getTotalMetrics(const vespalib::string &)
{
    LockGuard guard(_lock);
    return _totalMetrics;
}

} // namespace vespalib
