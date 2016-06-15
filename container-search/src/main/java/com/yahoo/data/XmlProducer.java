// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.data;

/**
 * Generic API for classes that contain data representable as XML.
 **/
public interface XmlProducer {

    /**
     * Append the XML representation of this object's data to a StringBuilder.
     * @param target the StringBuilder to append to.
     * @return the target passed in is also returned (to allow chaining).
     **/
    public StringBuilder writeXML(StringBuilder target);

    /**
     * Convenience method equivalent to:
     * makeXML(new StringBuilder()).toString()
     * @return String containing XML representation of this object's data.
     **/
    public String toXML();
}

