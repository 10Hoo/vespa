// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "span.h"
#include "spantreevisitor.h"

namespace document {

void Span::accept(SpanTreeVisitor &visitor) const { visitor.visit(*this); }

}  // namespace document
