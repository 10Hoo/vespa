// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
*******************************************************************************
*
* @author          Stein Hardy Danielsen
* @date            Creation date: 2000-1-14
* @version         $Id$
*
* @file
*
* Generic input stream interface
*
* Copyright (c)  : 1997-1999 Fast Search & Transfer ASA
*                  ALL RIGHTS RESERVED
*
******************************************************************************/
#pragma once

#include <vespa/fastos/fastos.h>



class Fast_InputStream
{
  public:

    virtual ~Fast_InputStream() { }

    virtual ssize_t Available(void) = 0;
    virtual bool    Close(void) = 0;
    virtual ssize_t Read(void *targetBuffer, size_t bufferSize) = 0;
    virtual ssize_t Skip(size_t skipNBytes) = 0;
};


