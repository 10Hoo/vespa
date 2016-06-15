// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/document/fieldset/fieldset.h>
#include <vespa/document/fieldset/fieldsets.h>
#include <vespa/document/datatype/documenttype.h>
#include <vespa/document/repo/documenttyperepo.h>

namespace document {

/**
 * Class that has configuration for all document field sets.
 * Responsible for parsing field set strings using a documenttype.
 */
class FieldSetRepo
{
public:
    static FieldSet::UP parse(const DocumentTypeRepo& repo,
                       const vespalib::stringref & fieldSetString);

    static vespalib::string serialize(const FieldSet& fs);
};

}


