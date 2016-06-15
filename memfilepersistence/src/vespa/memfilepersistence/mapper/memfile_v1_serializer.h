// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/memfilepersistence/mapper/bufferedfilewriter.h>
#include <vespa/memfilepersistence/mapper/versionserializer.h>
#include <vespa/memfilepersistence/mapper/fileinfo.h>
#include <vespa/memfilepersistence/mapper/simplememfileiobuffer.h>
#include <vespa/memfilepersistence/common/environment.h>
#include <vespa/memfilepersistence/spi/threadmetricprovider.h>

namespace storage {
namespace memfile {

class MemFileV1Serializer : public VersionSerializer
{
    ThreadMetricProvider& _metricProvider;
    MemFilePersistenceThreadMetrics& getMetrics() {
        return _metricProvider.getMetrics();
    }
public:
    typedef vespalib::LinkedPtr<MemFileV1Serializer> LP;

    MemFileV1Serializer(ThreadMetricProvider&);

    virtual FileVersion getFileVersion() { return TRADITIONAL_SLOTFILE; }

    virtual void loadFile(MemFile& file, Environment&,
                          Buffer& buffer, uint64_t bytesRead);

    void cacheLocationsForPart(SimpleMemFileIOBuffer& cache,
                               DocumentPart part,
                               uint32_t blockIndex,
                               const std::vector<DataLocation>& locationsToCache,
                               const std::vector<DataLocation>& locationsRead,
                               SimpleMemFileIOBuffer::BufferAllocation& buf);

    virtual void cacheLocations(MemFileIOInterface& cache,
                                Environment& env,
                                const Options& options,
                                DocumentPart part,
                                const std::vector<DataLocation>& locations);

    virtual FlushResult flushUpdatesToFile(MemFile&, Environment&);

    virtual void rewriteFile(MemFile&, Environment&);

    virtual bool verify(MemFile&, Environment&,
                        std::ostream& errorReport, bool repairErrors,
                        uint16_t fileVerifyFlags);

    uint64_t read(vespalib::LazyFile& file,
                  char* buf,
                  const std::vector<DataLocation>& readOps);

    void ensureFormatSpecificDataSet(const MemFile& file);

    uint32_t writeMetaData(BufferedFileWriter& writer,
                           const MemFile& file);

    uint32_t writeAndUpdateLocations(
            MemFile& file,
            SimpleMemFileIOBuffer& ioBuf,
            BufferedFileWriter& writer,
            DocumentPart part,
            const MemFile::LocationMap& locationsToWrite,
            const Environment& env);
};

} // memfile
} // storage

