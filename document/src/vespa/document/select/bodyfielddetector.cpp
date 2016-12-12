// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "bodyfielddetector.h"
#include <vespa/document/base/exceptions.h>
#include <vespa/document/repo/documenttyperepo.h>
#include <vespa/document/datatype/documenttype.h>
#include <vespa/vespalib/util/closure.h>
#include "valuenode.h"

namespace document {

namespace select {

void
BodyFieldDetector::detectFieldType(const FieldValueNode *expr,
                                   const DocumentType &type)
{
    if (type.getName() != expr->getDocType()) {
        return;
    }
    try {
        FieldPath::UP path(type.buildFieldPath(expr->getFieldName()));
        if (path.get() && path->size() != 0) {
            if ((*path)[0].getFieldRef().isHeaderField()) {
                foundHeaderField = true;
            } else {
                foundBodyField = true;
            }
        }
    } catch (FieldNotFoundException &) {
    }
}


void
BodyFieldDetector::visitFieldValueNode(const FieldValueNode& expr)
{
    _repo.forEachDocumentType(
            *makeClosure(this, &BodyFieldDetector::detectFieldType, &expr));
}


}  // namespace select
}  // namespace document
