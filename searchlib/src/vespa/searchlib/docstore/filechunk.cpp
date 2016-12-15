// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "filechunk.h"
#include "data_store_file_chunk_stats.h"
#include <vespa/searchlib/util/filekit.h>
#include <vespa/vespalib/data/fileheader.h>
#include <vespa/vespalib/stllike/asciistream.h>
#include <vespa/vespalib/util/array.hpp>
#include <vespa/vespalib/stllike/hash_map.hpp>

#include <vespa/log/log.h>
LOG_SETUP(".search.filechunk");

using vespalib::GenericHeader;
using vespalib::FileHeader;
using vespalib::IoException;
using vespalib::getLastErrorString;
using vespalib::getErrorString;


namespace search {

namespace {

constexpr size_t ALIGNMENT=0x1000;
constexpr size_t ENTRY_BIAS_SIZE=8;

}

using vespalib::make_string;

SummaryException::SummaryException(const vespalib::stringref &msg,
                                   FastOS_FileInterface &file,
                                   const vespalib::stringref &location)
    : IoException(make_string("%s : Failing file = '%s'. Reason given by OS = '%s'",
                              msg.c_str(), file.GetFileName(), file.getLastErrorString().c_str()),
                  getErrorType(file.GetLastError()), location)
{
}

FileChunk::ChunkInfo::ChunkInfo(uint64_t offset, uint32_t size, uint64_t lastSerial)
    : _lastSerial(lastSerial),
      _offset(offset),
      _size(size)
{
    assert(valid());
}


LidInfo::LidInfo(uint32_t fileId, uint32_t chunkId, uint32_t sz)
{
    _value.v.fileId = fileId;
    _value.v.chunkId = chunkId;
    _value.v.size = sz;
    if (fileId >= (1 << 10)) {
        throw std::runtime_error(
                make_string("LidInfo(fileId=%u, chunkId=%u, size=%u) has invalid fileId larger than %d",
                            fileId, chunkId, sz, (1 << 10) - 1));
    }
    if (chunkId >= (1 << 22)) {
        throw std::runtime_error(
                make_string("LidInfo(fileId=%u, chunkId=%u, size=%u) has invalid chunkId larger than %d",
                            fileId, chunkId, sz, (1 << 22) - 1));
    }
}

DirectIORandRead::DirectIORandRead(const vespalib::string & fileName)
    : _file(fileName.c_str()),
      _alignment(1),
      _granularity(1),
      _maxChunkSize(0x100000)
{
    _file.EnableDirectIO();
    if (_file.OpenReadOnly()) {
        if (!_file.GetDirectIORestrictions(_alignment, _granularity, _maxChunkSize)) {
            LOG(debug, "Direct IO setup failed for file %s due to %s",
                       _file.GetFileName(), _file.getLastErrorString().c_str());
        }
    } else {
        throw SummaryException("Failed opening data file", _file, VESPA_STRLOC);
    }
}

FileRandRead::FSP
DirectIORandRead::read(size_t offset, vespalib::DataBuffer & buffer, size_t sz)
{
    size_t padBefore(0);
    size_t padAfter(0);
    bool directio = _file.DirectIOPadding(offset, sz, padBefore, padAfter);
    buffer.clear();
    buffer.ensureFree(padBefore + sz + padAfter + _alignment - 1);
    if (directio) {
        size_t unAligned = (-reinterpret_cast<size_t>(buffer.getFree()) & (_alignment - 1));
        buffer.moveFreeToData(unAligned);
        buffer.moveDataToDead(unAligned);
    }
    // XXX needs to use pread or file-position-mutex
    _file.ReadBuf(buffer.getFree(), padBefore + sz + padAfter, offset - padBefore);
    buffer.moveFreeToData(padBefore + sz);
    buffer.moveDataToDead(padBefore);
    return FSP();
}


int64_t
DirectIORandRead::getSize(void)
{
    return _file.GetSize();
}


MMapRandRead::MMapRandRead(const vespalib::string & fileName, int mmapFlags, int fadviseOptions)
    : _file(fileName.c_str())
{
    _file.enableMemoryMap(mmapFlags);
    _file.setFAdviseOptions(fadviseOptions);
    if ( ! _file.OpenReadOnly()) {
        throw SummaryException("Failed opening data file", _file, VESPA_STRLOC);
    }
}


NormalRandRead::NormalRandRead(const vespalib::string & fileName)
    : _file(fileName.c_str())
{
    if ( ! _file.OpenReadOnly()) {
        throw SummaryException("Failed opening data file", _file, VESPA_STRLOC);
    }
}

FileRandRead::FSP
MMapRandRead::read(size_t offset, vespalib::DataBuffer & buffer, size_t sz)
{
    const char *ptr = static_cast<const char *>(_file.MemoryMapPtr(offset));
    vespalib::DataBuffer(ptr, sz).swap(buffer);
    return FSP();
}

int64_t
MMapRandRead::getSize(void)
{
    return _file.GetSize();
}

MMapRandReadDynamic::MMapRandReadDynamic(const vespalib::string &fileName, int mmapFlags, int fadviseOptions)
    : _fileName(fileName),
      _mmapFlags(mmapFlags),
      _fadviseOptions(fadviseOptions)
{
    reopen();
}

void
MMapRandReadDynamic::reopen()
{
    std::unique_ptr<FastOS_File> file(new FastOS_File(_fileName.c_str()));
    file->enableMemoryMap(_mmapFlags);
    file->setFAdviseOptions(_fadviseOptions);
    if (file->OpenReadOnly()) {
        _holder.set(file.release());
        _holder.latch();
    } else {
        throw SummaryException("Failed opening data file", *file, VESPA_STRLOC);
    }
}

FileRandRead::FSP
MMapRandReadDynamic::read(size_t offset, vespalib::DataBuffer & buffer, size_t sz)
{
    FSP file(_holder.get());
    const char * data(static_cast<const char *>(file->MemoryMapPtr(offset)));
    if ((data == NULL) || (file->MemoryMapPtr(offset+sz-1) == NULL)) {
        // Must check that both start and end of file is mapped in.
        // Previous reopen could happend during a partial write of this buffer.
        // This should fix bug 4630695.
        reopen();
        file = _holder.get();
        data = static_cast<const char *>(file->MemoryMapPtr(offset));
    }
    vespalib::DataBuffer(data, sz).swap(buffer);
    return file;
}

int64_t
MMapRandReadDynamic::getSize(void)
{
    return _holder.get()->GetSize();
}

FileRandRead::FSP
NormalRandRead::read(size_t offset, vespalib::DataBuffer & buffer, size_t sz)
{
    buffer.clear();
    buffer.ensureFree(sz);
    _file.ReadBuf(buffer.getFree(), sz, offset);
    buffer.moveFreeToData(sz);
    return FSP();
}

int64_t
NormalRandRead::getSize(void)
{
    return _file.GetSize();
}

vespalib::string
FileChunk::NameId::createName(const vespalib::string &baseName) const {
    vespalib::asciistream os;
    os << baseName << '/' << vespalib::setfill('0') << vespalib::setw(19) << getId();
    return os.str();
}

vespalib::string
FileChunk::createIdxFileName(const vespalib::string & name) {
    return name + ".idx";
}

vespalib::string
FileChunk::createDatFileName(const vespalib::string & name) {
    return name + ".dat";
}

FileChunk::FileChunk(FileId fileId, NameId nameId, const vespalib::string & baseName,
                     const TuneFileSummary & tune, const IBucketizer * bucketizer, bool skipCrcOnRead)
    : _fileId(fileId),
      _nameId(nameId),
      _name(nameId.createName(baseName)),
      _skipCrcOnRead(skipCrcOnRead),
      _entriesCount(0),
      _erasedCount(0),
      _erasedBytes(0),
      _diskFootprint(0),
      _sumNumBuckets(0),
      _numChunksWithBuckets(0),
      _numUniqueBuckets(0),
      _file(),
      _bucketizer(bucketizer),
      _addedBytes(0),
      _tune(tune),
      _dataFileName(createDatFileName(_name)),
      _idxFileName(createIdxFileName(_name)),
      _chunkInfo(),
      _dataHeaderLen(0u),
      _idxHeaderLen(0u),
      _lastPersistedSerialNum(0),
      _modificationTime()
{
    FastOS_File dataFile(_dataFileName.c_str());
    if (dataFile.OpenReadOnly()) {
        if (!dataFile.Sync()) {
            throw SummaryException("Failed syncing dat file", dataFile, VESPA_STRLOC);
        }
        _diskFootprint += dataFile.GetSize();
        FastOS_File idxFile(_idxFileName.c_str());
        if (idxFile.OpenReadOnly()) {
            if (!idxFile.Sync()) {
                throw SummaryException("Failed syncing idx file", idxFile, VESPA_STRLOC);
            }
            _diskFootprint += idxFile.GetSize();
            _modificationTime = FileKit::getModificationTime(_idxFileName);
        } else {
            dataFile.Close();
            throw SummaryException("Failed opening idx file", idxFile, VESPA_STRLOC);
        }
    } else {
    }
}

FileChunk::~FileChunk()
{
}

void
FileChunk::addNumBuckets(size_t numBucketsInChunk)
{
    _sumNumBuckets += numBucketsInChunk;
    if (numBucketsInChunk != 0) {
        ++_numChunksWithBuckets;
    }
}

class TmpChunkMeta : public ChunkMeta,
                     public std::vector<LidMeta>
{
public:
    void fill(vespalib::nbostream & is) {
        resize(getNumEntries());
        for (LidMeta & lm : *this) {
            lm.deserialize(is);
        }
    }
};

typedef vespalib::Array<TmpChunkMeta> TmpChunkMetaV;

namespace {

void
verifyOrAssert(const TmpChunkMetaV & v)
{
    for (auto prev(v.begin()), it(prev); it != v.end(); ++it) {
        assert(prev->getLastSerial() <= it->getLastSerial());
        prev = it;
    }
}

vespalib::string eraseErrorMsg(const vespalib::string & fileName, int error) {
    return make_string("Error erasing file '%s'. Error is '%s'",
                       fileName.c_str(), getErrorString(error).c_str());
}

}

void
FileChunk::erase()
{
    _file.reset();
    if (!FastOS_File::Delete(_idxFileName.c_str()) && (errno != ENOENT)) {
        throw std::runtime_error(eraseErrorMsg(_idxFileName, errno));
    }
    if (!FastOS_File::Delete(_dataFileName.c_str()) && (errno != ENOENT)) {
        throw std::runtime_error(eraseErrorMsg(_dataFileName, errno));
    }
}

size_t
FileChunk::updateLidMap(const LockGuard & guard, ISetLid & ds, uint64_t serialNum)
{
    size_t sz(0);
    assert(_chunkInfo.empty());

    FastOS_File idxFile(_idxFileName.c_str());
    idxFile.enableMemoryMap(0);
    if (idxFile.OpenReadOnly()) {
        if (idxFile.IsMemoryMapped()) {
            const int64_t fileSize = idxFile.GetSize();
            if (_idxHeaderLen == 0) {
                _idxHeaderLen = readIdxHeader(idxFile);
            }
            vespalib::nbostream is(static_cast<const char *>(idxFile.MemoryMapPtr(0)) + _idxHeaderLen,
                                   fileSize - _idxHeaderLen);
            TmpChunkMetaV tempVector;
            tempVector.reserve(fileSize/(sizeof(ChunkMeta)+sizeof(LidMeta)));
            while ( ! is.empty() && is.good()) {
                const int64_t lastKnownGoodPos = _idxHeaderLen + is.rp();
                tempVector.push_back(TmpChunkMeta());
                TmpChunkMeta & chunkMeta(tempVector.back());
                try {
                    chunkMeta.deserialize(is);
                    chunkMeta.fill(is);
                } catch (const vespalib::IllegalStateException & e) {
                    LOG(warning, "Exception deserializing idx file : %s", e.what());
                    LOG(warning, "File '%s' seems to be partially truncated. Will truncate from size=%ld to %ld",
                                 _idxFileName.c_str(), fileSize, lastKnownGoodPos);
                    FastOS_File toTruncate(_idxFileName.c_str());
                    if ( toTruncate.OpenReadWrite()) {
                        if (toTruncate.SetSize(lastKnownGoodPos)) {
                            tempVector.resize(tempVector.size() - 1);
                        } else {
                            throw SummaryException("SetSize(%ld) failed.", toTruncate, VESPA_STRLOC);
                        }
                    } else {
                        throw SummaryException("Open for truncation failed.", toTruncate, VESPA_STRLOC);
                    }
                }
            }
            if ( ! tempVector.empty()) {
                verifyOrAssert(tempVector);
                if (tempVector[0].getLastSerial() < serialNum) {
                    LOG(warning,
                        "last serial num(%ld) from previous file is "
                        "bigger than my first(%ld). That is odd."
                        "Current filename is '%s'",
                        serialNum, tempVector[0].getLastSerial(),
                        _idxFileName.c_str());
                    serialNum = tempVector[0].getLastSerial();
                }
                BucketDensityComputer globalBucketMap(_bucketizer);
                // Guard comes from the same bucketizer so the same guard can be used
                // for both local and global BucketDensityComputer
                vespalib::GenerationHandler::Guard bucketizerGuard = globalBucketMap.getGuard();
                for (const TmpChunkMeta & chunkMeta : tempVector) {
                    assert(serialNum <= chunkMeta.getLastSerial());
                    BucketDensityComputer bucketMap(_bucketizer);
                    for (size_t i(0), m(chunkMeta.getNumEntries()); i < m; i++) {
                        const LidMeta & lidMeta(chunkMeta[i]);
                        if (_bucketizer && (lidMeta.size() > 0)) {
                            document::BucketId bucketId = _bucketizer->getBucketOf(bucketizerGuard, lidMeta.getLid());
                            bucketMap.recordLid(bucketId);
                            globalBucketMap.recordLid(bucketId);
                        }
                        ds.setLid(guard, lidMeta.getLid(), LidInfo(getFileId().getId(), _chunkInfo.size(), lidMeta.size()));
                        incEntries();
                        _addedBytes += adjustSize(lidMeta.size());
                    }
                    serialNum = chunkMeta.getLastSerial();
                    addNumBuckets(bucketMap.getNumBuckets());
                    _chunkInfo.push_back(ChunkInfo(chunkMeta.getOffset(), chunkMeta.getSize(), chunkMeta.getLastSerial()));
                    assert(serialNum >= _lastPersistedSerialNum);
                    _lastPersistedSerialNum = serialNum;
                }
                _numUniqueBuckets = globalBucketMap.getNumBuckets();
            }
        } else {
            assert(idxFile.getSize() == 0);
        }
    } else {
        assert(false);
    }
    return sz;
}

void
FileChunk::enableRead()
{
    if (_tune._randRead.getWantDirectIO()) {
        LOG(debug, "enableRead(): DirectIORandRead: file='%s'", _dataFileName.c_str());
        _file.reset(new DirectIORandRead(_dataFileName));
    } else if (_tune._randRead.getWantMemoryMap()) {
        const int mmapFlags(_tune._randRead.getMemoryMapFlags());
        const int fadviseOptions(_tune._randRead.getAdvise());
        if (frozen()) {
            LOG(debug, "enableRead(): MMapRandRead: file='%s'", _dataFileName.c_str());
            _file.reset(new MMapRandRead(_dataFileName, mmapFlags, fadviseOptions));
        } else {
            LOG(debug, "enableRead(): MMapRandReadDynamic: file='%s'", _dataFileName.c_str());
            _file.reset(new MMapRandReadDynamic(_dataFileName, mmapFlags, fadviseOptions));
        }
    } else {
        LOG(debug, "enableRead(): NormalRandRead: file='%s'", _dataFileName.c_str());
        _file.reset(new NormalRandRead(_dataFileName));
    }
    _dataHeaderLen = readDataHeader(*_file);
    if (_dataHeaderLen == 0u) {
        throw std::runtime_error(make_string("bad file header: %s", _dataFileName.c_str()));
    }
}

size_t FileChunk::adjustSize(size_t sz) {
    return sz + ENTRY_BIAS_SIZE;
}
void
FileChunk::remove(uint32_t lid, uint32_t size)
{
     (void) lid;
     _erasedCount++;
     _erasedBytes += adjustSize(size);
}

uint64_t
FileChunk::getLastPersistedSerialNum() const
{
    return _lastPersistedSerialNum;
}

fastos::TimeStamp
FileChunk::getModificationTime() const
{
    return _modificationTime;
}

void
FileChunk::appendTo(const IGetLid & db, IWriteData & dest,
                    uint32_t numChunks,
                    IFileChunkVisitorProgress *visitorProgress)
{
    assert(frozen() || visitorProgress);
    vespalib::GenerationHandler::Guard lidReadGuard(db.getLidReadGuard());
    assert(numChunks <= getNumChunks());
    for (size_t chunkId(0); chunkId < numChunks; chunkId++) {
        const ChunkInfo & cInfo(_chunkInfo[chunkId]);
        vespalib::DataBuffer whole(0ul, ALIGNMENT);
        FileRandRead::FSP keepAlive(_file->read(cInfo.getOffset(), whole, cInfo.getSize()));
        Chunk chunk(chunkId, whole.getData(), whole.getDataLen());
        const Chunk::LidList ll(chunk.getUniqueLids());
        for (const Chunk::Entry & e : ll) {
            LidInfo lidInfo(getFileId().getId(), chunk.getId(), e.netSize());
            if (db.getLid(lidReadGuard, e.getLid()) == lidInfo) {
                vespalib::LockGuard guard(db.getLidGuard(e.getLid()));
                if (db.getLid(lidReadGuard, e.getLid()) == lidInfo) {
                    // I am still in use so I need to taken care of.
                    vespalib::ConstBufferRef data(chunk.getLid(e.getLid()));
                    dest.write(guard, chunk.getId(), e.getLid(), data.c_str(), data.size());
                }
            }
        }
        if (visitorProgress != NULL) {
            visitorProgress->updateProgress();
        }
    }
    dest.close();
}

void
FileChunk::read(LidInfoWithLidV::const_iterator begin, size_t count, IBufferVisitor & visitor) const
{
    if (count == 0) { return; }
    uint32_t prevChunk = begin->getChunkId();
    uint32_t start(0);
    for (size_t i(0); i < count; i++) {
        const LidInfoWithLid & li = *(begin + i);
        if (li.getChunkId() != prevChunk) {
            ChunkInfo ci = _chunkInfo[prevChunk];
            read(begin + start, i - start, ci, visitor);
            prevChunk = li.getChunkId();
            start = i;
        }
    }
    ChunkInfo ci = _chunkInfo[prevChunk];
    read(begin + start, count - start, ci, visitor);
}

void
FileChunk::read(LidInfoWithLidV::const_iterator begin, size_t count, ChunkInfo ci, IBufferVisitor & visitor) const
{
    vespalib::DataBuffer whole(0ul, ALIGNMENT);
    FileRandRead::FSP keepAlive = _file->read(ci.getOffset(), whole, ci.getSize());
    Chunk chunk(begin->getChunkId(), whole.getData(), whole.getDataLen(), _skipCrcOnRead);
    for (size_t i(0); i < count; i++) {
        const LidInfoWithLid & li = *(begin + i);
        vespalib::ConstBufferRef buf = chunk.getLid(li.getLid());
        if (buf.size() != 0) {
            visitor.visit(li.getLid(), buf);
        }
    }
}

ssize_t
FileChunk::read(uint32_t lid, SubChunkId chunkId,
                vespalib::DataBuffer & buffer) const
{
    return (chunkId < _chunkInfo.size())
        ? read(lid, chunkId, _chunkInfo[chunkId], buffer)
        : -1;
}

ssize_t
FileChunk::read(uint32_t lid, SubChunkId chunkId, const ChunkInfo & chunkInfo,
                vespalib::DataBuffer & buffer) const
{
    vespalib::DataBuffer whole(0ul, ALIGNMENT);
    FileRandRead::FSP keepAlive(_file->read(chunkInfo.getOffset(), whole, chunkInfo.getSize()));
    Chunk chunk(chunkId, whole.getData(), whole.getDataLen(), _skipCrcOnRead);
    return chunk.read(lid, buffer);
}

uint64_t
FileChunk::readDataHeader(FileRandRead &datFile)
{
    uint64_t dataHeaderLen(0);
    int64_t fileSize = datFile.getSize();
    uint32_t hl = GenericHeader::getMinSize();
    if (fileSize >= hl) {
        vespalib::DataBuffer h(hl, ALIGNMENT);
        datFile.read(0, h, hl);
        GenericHeader::BufferReader rd(h);
        uint32_t headerLen = GenericHeader::readSize(rd);
        if (headerLen <= fileSize) {
            dataHeaderLen = headerLen;
        }
    }
    return dataHeaderLen;
}


uint64_t
FileChunk::readIdxHeader(FastOS_FileInterface &idxFile)
{
    int64_t fileSize = idxFile.GetSize();
    uint32_t hl = GenericHeader::getMinSize();
    uint64_t idxHeaderLen = 0;
    if (fileSize >= hl) {
        GenericHeader::MMapReader rd(static_cast<const char *> (idxFile.MemoryMapPtr(0)), hl);
        uint32_t headerLen = GenericHeader::readSize(rd);
        if (headerLen <= fileSize) {
            idxHeaderLen = headerLen;
        }
    }
    if (idxHeaderLen == 0u) {
        throw SummaryException("bad file header", idxFile, VESPA_STRLOC);
    }
    return idxHeaderLen;
}

void
FileChunk::verify(bool reportOnly) const
{
    (void) reportOnly;
    LOG(info,
        "Verifying file '%s' with fileid '%u'. erased-count='%zu' and erased-bytes='%zu'. diskFootprint='%zu'",
        _name.c_str(), _fileId.getId(), _erasedCount, _erasedBytes, _diskFootprint);
    size_t lastSerial(0);
    size_t chunkId(0);
    bool errorInPrev(false);
    for (const ChunkInfo & ci : _chunkInfo) {
        vespalib::DataBuffer whole(0ul, ALIGNMENT);
        FileRandRead::FSP keepAlive(_file->read(ci.getOffset(), whole, ci.getSize()));
        try {
            Chunk chunk(chunkId++, whole.getData(), whole.getDataLen());
            assert(chunk.getLastSerial() >= lastSerial);
            lastSerial = chunk.getLastSerial();
            if (errorInPrev) {
                LOG(error, "Last serial number in first good chunk is %ld", chunk.getLastSerial());
                errorInPrev = false;
            }
        } catch (const std::exception & e) {
            LOG(error,
                "Errors in chunk number %ld/%ld at file offset %lu and size %u."
                " Last known good serial number = %ld\n.Got Exception : %s",
                chunkId, _chunkInfo.size(), ci.getOffset(), ci.getSize(), lastSerial, e.what());
            errorInPrev = true;
        }
    }
}

uint32_t
FileChunk::getNumChunks() const
{
    return _chunkInfo.size();
}

size_t
FileChunk::getMemoryFootprint() const
{
    // The memory footprint does not vary before or after flush
    // Once frozen, there is no variable component.
    // It is all captured by getMemoryMetaFootprint()
    return 0;
}   
    
size_t
FileChunk::getMemoryMetaFootprint() const
{
    return sizeof(*this) + _chunkInfo.byteSize();
}

bool
FileChunk::isIdxFileEmpty(const vespalib::string & name)
{
    vespalib::string fileName(name + ".idx");
    FastOS_File idxFile(fileName.c_str());
    idxFile.enableMemoryMap(0);
    if (idxFile.OpenReadOnly()) {
        if (idxFile.IsMemoryMapped()) {
            int64_t fileSize = idxFile.getSize();
            int64_t idxHeaderLen = FileChunk::readIdxHeader(idxFile);
            return fileSize <= idxHeaderLen;
        } else if ( idxFile.getSize() == 0u) {
            return true;
        } else {
            throw SummaryException("Failed opening idx file for memorymapping", idxFile, VESPA_STRLOC);
        }
    } else {
        throw SummaryException("Failed opening idx file readonly ", idxFile, VESPA_STRLOC);
    }
    return false;
}

void
FileChunk::eraseIdxFile(const vespalib::string & name)
{
    vespalib::string fileName(name + ".idx");
    if ( ! FastOS_File::Delete(fileName.c_str())) {
        throw std::runtime_error(make_string("Failed to delete '%s'", fileName.c_str()));
    }
}


DataStoreFileChunkStats
FileChunk::getStats() const
{
    uint64_t diskFootprint = getDiskFootprint();
    uint64_t diskBloat = getDiskBloat();
    double bucketSpread = getBucketSpread();
    uint64_t serialNum = getLastPersistedSerialNum();
    uint64_t nameId = getNameId().getId();
    return DataStoreFileChunkStats(diskFootprint, diskBloat, bucketSpread,
                                   serialNum, serialNum, nameId);
}

} // namespace search
