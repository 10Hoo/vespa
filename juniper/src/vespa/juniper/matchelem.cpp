// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/* $Id$ */

#include <vespa/fastos/fastos.h>
#include "matchelem.h"

MatchElement::MatchElement(off_t spos, off_t stoken) :
    _starttoken(stoken),
    _startpos(spos),
    _valid(false)
{ }
