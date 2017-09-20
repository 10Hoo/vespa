// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/*
 * Author: Stein Hardy Danielsen
 */

#include "fileoutputstream.h"
#include <vespa/fastos/file.h>


Fast_FileOutputStream::Fast_FileOutputStream(const char *fileName)
    : _theFile(new FastOS_File(fileName))
{
    _theFile->OpenWriteOnly();
}


Fast_FileOutputStream::~Fast_FileOutputStream(void)
{
    _theFile->Close();
    delete _theFile;
}

ssize_t
Fast_FileOutputStream::Write(const void *sourceBuffer, size_t bufferSize) {
    return _theFile->CheckedWrite(sourceBuffer, bufferSize) ?
           static_cast<ssize_t>(bufferSize) :
           static_cast<ssize_t>(-1);
};

bool
Fast_FileOutputStream::Close() {
    return _theFile->Close();
}
