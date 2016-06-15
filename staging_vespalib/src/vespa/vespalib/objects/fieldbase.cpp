// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include "fieldbase.h"

namespace vespalib {

FieldBase SerializerCommon::_unspecifiedField("unspecified");
FieldBase SerializerCommon::_sizeField("size");

}
