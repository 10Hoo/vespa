// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".locationiterators");
#include <vespa/vespalib/geo/zcurve.h>

#include <vespa/searchlib/bitcompression/compression.h>

#include "locationiterators.h"

using namespace search::common;

class FastS_2DZLocationIterator : public search::queryeval::SearchIterator, public vespalib::noncopyable
{
private:
    const unsigned int _numDocs;
    const bool         _strict;
    const uint64_t     _radius2;
    const Location &   _location;
    std::vector<search::AttributeVector::largeint_t> _pos;

    virtual void doSeek(uint32_t docId);
    virtual void doUnpack(uint32_t docId);
public:
    FastS_2DZLocationIterator(unsigned int numDocs,
                             bool strict,
                             const Location & location);

    virtual ~FastS_2DZLocationIterator(void);
};


FastS_2DZLocationIterator::
FastS_2DZLocationIterator(unsigned int numDocs,
                          bool strict,
                          const Location & location)
    : SearchIterator(),
      _numDocs(numDocs),
      _strict(strict),
      _radius2(static_cast<uint64_t>(location.getRadius()) * location.getRadius()),
      _location(location),
      _pos()
{
    _pos.resize(1);  //Need at least 1 entry as the singlevalue attributes does not honour given size.
};


FastS_2DZLocationIterator::~FastS_2DZLocationIterator(void)
{
};


void
FastS_2DZLocationIterator::doSeek(uint32_t docId)
{
    if (__builtin_expect(docId >= _numDocs, false)) {
        setAtEnd();
        return;
    }

    const Location &location = _location;
    std::vector<search::AttributeVector::largeint_t> &pos = _pos;

    for (;;) {
        uint32_t numValues =
            location.getVec()->get(docId, &pos[0], pos.size());
        if (numValues > pos.size()) {
            pos.resize(numValues);
            numValues = location.getVec()->get(docId, &pos[0], pos.size());
        }
        for (uint32_t i = 0; i < numValues; i++) {
            int64_t docxy(pos[i]);
            if ( ! location.getzFailBoundingBoxTest(docxy)) {
                int32_t docx = 0;
                int32_t docy = 0;
                vespalib::geo::ZCurve::decode(docxy, &docx, &docy);
                uint32_t dx = (location.getX() > docx)
                              ? location.getX() - docx
                              : docx - location.getX();
                if (location.getXAspect() != 0)
                    dx = ((uint64_t) dx * location.getXAspect()) >> 32;

                uint32_t dy = (location.getY() > docy)
                              ? location.getY() - docy
                              : docy - location.getY();
                uint64_t dist2 = (uint64_t) dx * dx + (uint64_t) dy * dy;
                if (dist2 <= _radius2) {
                    setDocId(docId);
                    return;
                }
            }
        }

        if (__builtin_expect(docId + 1 >= _numDocs, false)) {
            setAtEnd();
            return;
        }

        if (!_strict) {
            return;
        }
        docId++;
    }
}


void
FastS_2DZLocationIterator::doUnpack(uint32_t docId)
{
    (void) docId;
}


search::queryeval::SearchIterator *
FastS_AllocLocationIterator(unsigned int numDocs,
                            bool strict,
                            const Location & location)
{
    return new FastS_2DZLocationIterator(numDocs, strict, location);
}
