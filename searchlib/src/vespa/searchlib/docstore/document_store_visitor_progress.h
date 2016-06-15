// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "idocumentstore.h"

namespace search
{

class DocumentStoreVisitorProgress : public IDocumentStoreVisitorProgress
{
    double _progress;
public:
    DocumentStoreVisitorProgress();

    virtual void
    updateProgress(double progress);

    virtual double
    getProgress() const;
};

} // namespace proton

