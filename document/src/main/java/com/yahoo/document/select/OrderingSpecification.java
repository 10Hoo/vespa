// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.select;

public class OrderingSpecification {
    public static int ASCENDING = 0;
    public static int DESCENDING = 1;

    public int order;
    public long orderingStart;
    public short widthBits;
    public short divisionBits;

    public OrderingSpecification() {
        this(ASCENDING, (long)0, (short)0, (short)0);
    }

    public OrderingSpecification(int order) {
        this(order, (long)0, (short)0, (short)0);
    }

    public OrderingSpecification(int order, long orderingStart, short widthBits, short divisionBits) {
        this.order = order;
        this.orderingStart = orderingStart;
        this.widthBits = widthBits;
        this.divisionBits = divisionBits;
    }

    public int getOrder() { return order; }
    public long getOrderingStart() { return orderingStart; }
    public short getWidthBits() { return widthBits; }
    public short getDivisionBits() { return divisionBits; }

    public boolean equals(Object other) {
        OrderingSpecification o = (OrderingSpecification)other;
        if (o == null) return false;

        return (order == o.order && orderingStart == o.orderingStart && widthBits == o.widthBits && divisionBits == o.divisionBits);
    }

    public String toString() {
        return "O: " + order + " S:" + orderingStart + " W:" + widthBits + " D:" + divisionBits;
    }
}
