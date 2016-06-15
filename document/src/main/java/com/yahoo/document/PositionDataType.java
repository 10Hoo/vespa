// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document;

import com.yahoo.document.datatypes.FieldValue;
import com.yahoo.document.datatypes.IntegerFieldValue;
import com.yahoo.document.datatypes.Struct;
import com.yahoo.geo.DegreesParser;
import com.yahoo.document.serialization.XmlStream;

/**
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 */
public final class PositionDataType {

    public static final StructDataType INSTANCE = newInstance();
    public static final String STRUCT_NAME = "position";

    public static final String FIELD_X = "x";
    public static final String FIELD_Y = "y";
    private static final Field FFIELD_X = INSTANCE.getField(FIELD_X);
    private static final Field FFIELD_Y = INSTANCE.getField(FIELD_Y);

    private PositionDataType() {
        // unreachable
    }

    public static String renderAsString(Struct pos) {
        StringBuilder buf = new StringBuilder();
        double ns = getYValue(pos).getInteger() / 1.0e6;
        double ew = getXValue(pos).getInteger() / 1.0e6;
        buf.append(ns < 0 ? "S" : "N");
        buf.append(ns < 0 ? (-ns) : ns);
        buf.append(";");
        buf.append(ew < 0 ? "W" : "E");
        buf.append(ew < 0 ? (-ew) : ew);
        return buf.toString();
    }

    public static void renderXml(Struct pos, XmlStream target) {
        target.addContent(renderAsString(pos));
    }

    public static Struct valueOf(Integer x, Integer y) {
        Struct ret = new Struct(INSTANCE);
        ret.setFieldValue(FIELD_X, x != null ? new IntegerFieldValue(x) : null);
        ret.setFieldValue(FIELD_Y, y != null ? new IntegerFieldValue(y) : null);
        return ret;
    }

    public static Struct fromLong(long val) {
        return valueOf((int)(val >> 32), (int)val);
    }

    public static Struct fromString(String str) {
        try {
            DegreesParser d = new DegreesParser(str);
            return valueOf((int)(d.longitude * 1000000), (int)(d.latitude * 1000000));
        } catch (IllegalArgumentException e) {
            // empty
        }
        String[] arr = str.split(";", 2);
        return valueOf(Integer.parseInt(arr[0]), Integer.parseInt(arr[1]));
    }

    public static IntegerFieldValue getXValue(FieldValue pos) {
        return Struct.getFieldValue(pos, INSTANCE, FFIELD_X, IntegerFieldValue.class);
    }

    public static IntegerFieldValue getYValue(FieldValue pos) {
        return Struct.getFieldValue(pos, INSTANCE, FFIELD_Y, IntegerFieldValue.class);
    }

    public static String getZCurveFieldName(String fieldName) {
        return fieldName + "_zcurve";
    }

    public static String getPositionSummaryFieldName(String fieldName) {
        // TODO for 6.0, rename to _position to use a field name that is actually legal
        return fieldName + ".position";
    }

    public static String getDistanceSummaryFieldName(String fieldName) {
        // TODO for 6.0, rename to _distance to use a field name that is actually legal
        return fieldName + ".distance";
    }

    private static StructDataType newInstance() {
        StructDataType ret = new StructDataType(STRUCT_NAME);
        ret.addField(new Field(FIELD_X, DataType.INT));
        ret.addField(new Field(FIELD_Y, DataType.INT));
        return ret;
    }
}
