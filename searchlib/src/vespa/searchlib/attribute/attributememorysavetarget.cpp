// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".searchlib.attribute.attributememorysavetarget");

#include "attributememorysavetarget.h"
#include "attributefilesavetarget.h"
#include "attributevector.h"

namespace search
{

using search::common::FileHeaderContext;

AttributeMemorySaveTarget::AttributeMemorySaveTarget()
    : _datWriter(),
      _idxWriter(),
      _weightWriter(),
      _udatWriter()
{
}


IAttributeFileWriter &
AttributeMemorySaveTarget::datWriter()
{
    return _datWriter;
}


IAttributeFileWriter &
AttributeMemorySaveTarget::idxWriter()
{
    return _idxWriter;
}


IAttributeFileWriter &
AttributeMemorySaveTarget::weightWriter()
{
    return _weightWriter;
}


IAttributeFileWriter &
AttributeMemorySaveTarget::udatWriter()
{
    return _udatWriter;
}


bool
AttributeMemorySaveTarget::
writeToFile(const TuneFileAttributes &tuneFileAttributes,
            const FileHeaderContext &fileHeaderContext)
{
    AttributeFileSaveTarget saveTarget(tuneFileAttributes, fileHeaderContext);
    saveTarget.setConfig(_cfg);
    if (!saveTarget.setup()) {
        return false;
    }
    _datWriter.writeTo(saveTarget.datWriter());
    if (_cfg.getEnumerated()) {
        _udatWriter.writeTo(saveTarget.udatWriter());
    }
    if (_cfg.hasMultiValue()) {
        _idxWriter.writeTo(saveTarget.idxWriter());
        if (_cfg.hasWeightedSetType()) {
            _weightWriter.writeTo(saveTarget.weightWriter());
        }
    }
    saveTarget.close();
    return true;
}

} // namespace search

