// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.tensor;

import org.junit.Test;

import static org.hamcrest.Matchers.containsString;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.fail;

/**
 * @author geirst
 */
public class TensorTypeTestCase {

    @Test
    public void requireThatAnEmptyTensorTypeCanBeSpecified() {
        assertTensorType("tensor()");
    }

    @Test
    public void requireThatBoundIndexedDimensionsCanBeSpecified() {
        assertTensorType("tensor(x[5])");
        assertTensorType("tensor(x[5],y[10],z[100])");
        assertTensorType("tensor(x[5],y[10],z[100])", "tensor( x[5] , y[10] , z[100] )");
        assertTensorType("tensor(baR_09[10])");
    }

    @Test
    public void requireThatUnboundIndexedDimensionsCanBeSpecified() {
        assertTensorType("tensor(x[])");
        assertTensorType("tensor(x[],y[],z[])");
        assertTensorType("tensor(x[],y[],z[])", "tensor( x[] , y[] , z[] )");
        assertTensorType("tensor(baR_09[])");
    }

    @Test
    public void requireThatMappedDimensionsCanBeSpecified() {
        assertTensorType("tensor(x{})");
        assertTensorType("tensor(x{},y{},z{})");
        assertTensorType("tensor(x{},y{},z{})", "tensor( x{} , y{} , z{} )");
        assertTensorType("tensor(baR_09{})");
    }

    @Test
    public void requireThatIndexedBoundDimensionMustHaveNonZeroSize() {
        assertIllegalTensorType("tensor(x[0])", "Size of bound dimension 'x' must be at least 1");
    }

    @Test
    public void requireThatDimensionsMustHaveUniqueNames() {
        assertIllegalTensorType("tensor(x[10],y[20],x[30])", "Could not add dimension x[30] as this dimension is already present");
        assertIllegalTensorType("tensor(x{},y{},x{})", "Could not add dimension x{} as this dimension is already present");
    }

    @Test
    public void requireThatDimensionsAreOfSameType() {
        assertIllegalTensorType("tensor(x[],y{})", "[x[], y{}] contains both indexed and mapped dimensions, this is not supported yet");
    }

    @Test
    public void requireThatIllegalSyntaxInSpecThrowsException() {
        assertIllegalTensorType("foo(x[10])", "Tensor type spec must start with 'tensor(' and end with ')', but was 'foo(x[10])'");
        assertIllegalTensorType("tensor(x_@[10])", "Failed parsing element 'x_@[10]' in type spec 'tensor(x_@[10])'");
        assertIllegalTensorType("tensor(x[10a])", "Failed parsing element 'x[10a]' in type spec 'tensor(x[10a])'");
        assertIllegalTensorType("tensor(x{10})", "Failed parsing element 'x{10}' in type spec 'tensor(x{10})'");
    }

    private static void assertTensorType(String typeSpec) {
        assertTensorType(typeSpec, typeSpec);
    }

    private static void assertTensorType(String expected, String typeSpec) {
        assertEquals(expected, TensorType.fromSpec(typeSpec).toString());
    }

    private static void assertIllegalTensorType(String typeSpec, String messageSubstring) {
        try {
            TensorType.fromSpec(typeSpec);
            fail("Expoected exception to be thrown with message: '" + messageSubstring + "'");
        } catch (IllegalArgumentException e) {
            assertThat(e.getMessage(), containsString(messageSubstring));
        }
    }

}
