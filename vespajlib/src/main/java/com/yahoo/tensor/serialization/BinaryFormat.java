// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.tensor.serialization;

import com.google.common.annotations.Beta;
import com.yahoo.io.GrowableByteBuffer;
import com.yahoo.tensor.Tensor;

/**
 * Representation of a specific binary format with functions for serializing a Tensor object into
 * this format or de-serializing binary data into a Tensor object.
 *
 * @author <a href="mailto:geirst@yahoo-inc.com">Geir Storli</a>
 */
@Beta
interface BinaryFormat {

    /**
     * Serialize the given tensor into binary format.
     */
    public void encode(GrowableByteBuffer buffer, Tensor tensor);

    /**
     * Deserialize the given binary data into a Tensor object.
     */
    public Tensor decode(GrowableByteBuffer buffer);
}
