// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/*
 * Generic filter input stream
 *
 * Author: Stein Hardy Danielsen
 */
#pragma once

#include "inputstream.h"

class Fast_FilterInputStream : public Fast_InputStream
{
private:

    // Prevent use of:
    Fast_FilterInputStream(void);
    Fast_FilterInputStream(Fast_FilterInputStream &);
    Fast_FilterInputStream &operator=(const Fast_FilterInputStream &);


protected:

    /** The stream to forward data to */
    Fast_InputStream *_in;


public:

    // Constructors
    Fast_FilterInputStream(Fast_InputStream &in) : _in(&in) {}

    ~Fast_FilterInputStream() {};

    ssize_t Available()              override  { return _in->Available();      }
    bool    Close()                  override  { return _in->Close();          }
    ssize_t Skip(size_t skipNBytes)  override { return _in->Skip(skipNBytes); }

    ssize_t Read(void *targetBuffer, size_t length) override {
        return _in->Read(targetBuffer, length);
    }
};
