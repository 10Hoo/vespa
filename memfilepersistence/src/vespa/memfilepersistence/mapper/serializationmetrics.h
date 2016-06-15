// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/metrics/metrics.h>

namespace storage {
namespace memfile {

class SerializationWriteMetrics : public metrics::MetricSet
{
public:
    metrics::LongAverageMetric headerLatency;
    metrics::LongAverageMetric headerSize;
    metrics::LongAverageMetric bodyLatency;
    metrics::LongAverageMetric bodySize;
    metrics::LongAverageMetric metaLatency;
    metrics::LongAverageMetric metaSize;
    metrics::LongAverageMetric totalLatency;

    SerializationWriteMetrics(const std::string& name, metrics::MetricSet& owner)
        : metrics::MetricSet(name, "",
                             "Write metrics for memfile persistence engine",
                             &owner),
        headerLatency("header_latency", "",
                      "Time spent writing a single contiguous header location "
                      "on the disk.", this),
        headerSize("header_size", "",
                   "Average size of contiguous header disk writes", this),
        bodyLatency("body_latency", "",
                    "Time spent writing a single contiguous body location "
                    "on the disk.", this),
        bodySize("body_size", "",
                 "Average size of contiguous body disk writes", this),
        metaLatency("meta_latency", "",
                    "Time spent writing file header and slot metadata", this),
        metaSize("meta_size", "",
                 "Size of file header and metadata writes", this),
        totalLatency("total_latency", "",
                     "Total time spent performing slot file writing", this)
    {
    }
};

class SerializationMetrics : public metrics::MetricSet
{
public:
    metrics::LongAverageMetric initialMetaReadLatency;
    metrics::LongAverageMetric tooLargeMetaReadLatency;
    metrics::LongAverageMetric totalLoadFileLatency;
    metrics::LongAverageMetric verifyLatency;
    metrics::LongAverageMetric deleteFileLatency;
    metrics::LongAverageMetric headerReadLatency;
    metrics::LongAverageMetric headerReadSize;
    metrics::LongAverageMetric bodyReadLatency;
    metrics::LongAverageMetric bodyReadSize;
    metrics::LongAverageMetric cacheUpdateAndImplicitVerifyLatency;
    metrics::LongCountMetric fullRewritesDueToDownsizingFile;
    metrics::LongCountMetric fullRewritesDueToTooSmallFile;
    SerializationWriteMetrics partialWrite;
    SerializationWriteMetrics fullWrite;

    SerializationMetrics(const std::string& name,
                         metrics::MetricSet* owner = 0)
        : metrics::MetricSet(name, "",
                             "(De-)serialization I/O metrics for memfile "
                             "persistence engine", owner),
        initialMetaReadLatency(
                "initial_meta_read_latency", "",
                "Time spent doing the initial read of "
                "the file header and most (or all) of metadata",
                this),
        tooLargeMetaReadLatency(
                "too_large_meta_read_latency", "",
                "Time spent doing additional read for "
                "metadata too large to be covered by initial "
                "read", this),
        totalLoadFileLatency(
                "total_load_file_latency", "",
                "Total time spent initially loading a "
                "file from disk", this),
        verifyLatency(
                "verify_latency", "",
                "Time spent performing file verification", this),
        deleteFileLatency(
                "delete_file_latency", "",
                "Time spent deleting a file from disk", this),
        headerReadLatency(
                "header_read_latency", "",
                "Time spent reading a single contiguous header location "
                "on the disk (may span many document blobs)", this),
        headerReadSize(
                "header_read_size", "",
                "Size of contiguous header disk location reads", this),
        bodyReadLatency(
                "body_read_latency", "",
                "Time spent reading a single contiguous body location "
                "on the disk (may span many document blobs)", this),
        bodyReadSize(
                "body_read_size", "",
                "Size of contiguous body disk location reads", this),
        cacheUpdateAndImplicitVerifyLatency(
                "cache_update_and_implicit_verify_latency", "",
                "Time spent updating memory cache structures and verifying "
                "read data blocks for corruptions", this),
        fullRewritesDueToDownsizingFile(
                "full_rewrites_due_to_downsizing_file", "",
                "Number of times a file was rewritten fully because the "
                "original file had too low fill rate", this),
        fullRewritesDueToTooSmallFile(
                "full_rewrites_due_to_too_small_file", "",
                "Number of times a file was rewritten fully because the "
                "original file did not have sufficient free space for a "
                "partial write", this),
        partialWrite("partialwrite", *this),
        fullWrite("fullwrite", *this)
    {
    }
};

} // memfile
} // storage
