package com.yahoo.tensor;

import com.google.common.collect.ImmutableList;
import com.yahoo.tensor.evaluation.MapEvaluationContext;
import com.yahoo.tensor.evaluation.VariableTensor;
import com.yahoo.tensor.functions.ConstantTensor;
import com.yahoo.tensor.functions.Join;
import com.yahoo.tensor.functions.Reduce;
import com.yahoo.tensor.functions.TensorFunction;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import static org.junit.Assert.assertEquals;
import static com.yahoo.tensor.TensorType.Dimension.Type;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * Tests Tensor functionality
 * 
 * @author bratseth
 */
public class TensorTestCase {

    @Test
    public void testStringForm() {
        assertEquals("{}", Tensor.from("{}").toString());
        assertEquals("{{d1:l1,d2:l1}:5.0,{d1:l1,d2:l2}:6.0}", Tensor.from("{ {d1:l1,d2:l1}: 5,   {d2:l2, d1:l1}:6.0} ").toString());
        assertEquals("{{d1:l1,d2:l1}:-5.3,{d1:l1,d2:l2}:0.0}", Tensor.from("{ {d1:l1,d2:l1}:-5.3, {d2:l2, d1:l1}:0}").toString());
    }

    @Test
    public void testParseError() {
        try {
            Tensor.from("--");
            fail("Expected parse error");
        }
        catch (IllegalArgumentException expected) {
            assertEquals("Excepted a number or a string starting by { or tensor(, got '--'", expected.getMessage());
        }
    }

    @Test
    public void testDimensions() {
        Set<String> dimensions1 = Tensor.from("{} ").type().dimensionNames();
        assertEquals(0, dimensions1.size());

        Set<String> dimensions2 = Tensor.from("{ {d1:l1, d2:l2}:5, {d1:l2, d2:l2}:6.0} ").type().dimensionNames();
        assertEquals(2, dimensions2.size());
        assertTrue(dimensions2.contains("d1"));
        assertTrue(dimensions2.contains("d2"));

        Set<String> dimensions3 = Tensor.from("{ {d1:l1, d2:l1, d3:l1}:5, {d1:l1, d2:l2, d3:l1}:6.0} ").type().dimensionNames();
        assertEquals(3, dimensions3.size());
        assertTrue(dimensions3.contains("d1"));
        assertTrue(dimensions3.contains("d2"));
        assertTrue(dimensions3.contains("d3"));
    }

    /** All functions are more throughly tested in searchlib EvaluationTestCase */
    @Test
    public void testTensorComputation() {
        Tensor tensor1 = Tensor.from("{ {x:1}:3, {x:2}:7 }");
        Tensor tensor2 = Tensor.from("{ {y:1}:5 }");
        assertEquals(Tensor.from("{ {x:1,y:1}:15, {x:2,y:1}:35 }"), tensor1.multiply(tensor2));
        assertEquals(Tensor.from("{ {x:1,y:1}:12, {x:2,y:1}:28 }"), tensor1.join(tensor2, (a, b) -> a * b - a ));
        assertEquals(Tensor.from("{ {x:1,y:1}:0, {x:2,y:1}:1 }"), tensor1.larger(tensor2));
        assertEquals(Tensor.from("{ {y:1}:50.0 }"), tensor1.matmul(tensor2, "x"));
        assertEquals(Tensor.from("{ {z:1}:3, {z:2}:7 }"), tensor1.rename("x", "z"));
        assertEquals(Tensor.from("{ {y:1,x:1}:8, {x:1,y:2}:12 }"), tensor1.add(tensor2).rename(ImmutableList.of("x", "y"),
                                                                                               ImmutableList.of("y", "x")));
    }
    
    /** Test the same computation made in various ways which are implemented with special-cvase optimizations */
    @Test
    public void testOptimizedComputation() {
        assertEquals("Mapped vector",  42, (int)dotProduct(vector(Type.mapped), vectors(Type.mapped, 2)));
        assertEquals("Indexed vector", 42, (int)dotProduct(vector(Type.indexedUnbound), vectors(Type.indexedUnbound, 2)));
        assertEquals("Mapped matrix",  42, (int)dotProduct(vector(Type.mapped), matrix(Type.mapped, 2)));
        assertEquals("Indexed matrix", 42, (int)dotProduct(vector(Type.indexedUnbound), matrix(Type.indexedUnbound, 2)));
        assertEquals("Mixed vector",   42, (int)dotProduct(vector(Type.mapped), vectors(Type.indexedUnbound, 2)));
        assertEquals("Mixed vector",   42, (int)dotProduct(vector(Type.mapped), vectors(Type.indexedUnbound, 2)));
        assertEquals("Mixed matrix",   42, (int)dotProduct(vector(Type.mapped), matrix(Type.indexedUnbound, 2)));
        assertEquals("Mixed matrix",   42, (int)dotProduct(vector(Type.mapped), matrix(Type.indexedUnbound, 2)));
        assertEquals("Mixed vector",   42, (int)dotProduct(vector(Type.indexedUnbound), vectors(Type.mapped, 2)));
        assertEquals("Mixed vector",   42, (int)dotProduct(vector(Type.indexedUnbound), vectors(Type.mapped, 2)));
        assertEquals("Mixed matrix",   42, (int)dotProduct(vector(Type.indexedUnbound), matrix(Type.mapped, 2)));
        assertEquals("Mixed matrix",   42, (int)dotProduct(vector(Type.indexedUnbound), matrix(Type.mapped, 2)));
        
        // Test the unoptimized path by joining in another dimension
        Tensor unitJ = Tensor.Builder.of(new TensorType.Builder().mapped("j").build()).cell().label("j", 0).value(1).build();
        Tensor unitK = Tensor.Builder.of(new TensorType.Builder().mapped("k").build()).cell().label("k", 0).value(1).build();
        Tensor vectorInJSpace = vector(Type.mapped).multiply(unitJ);
        Tensor matrixInKSpace = matrix(Type.mapped, 2).get(0).multiply(unitK);
        assertEquals("Generic computation implementation", 42, (int)dotProduct(vectorInJSpace, Collections.singletonList(matrixInKSpace)));
    }

    private double dotProduct(Tensor tensor, List<Tensor> tensors) {
        double sum = 0;
        TensorFunction dotProductFunction = new Reduce(new Join(new ConstantTensor(tensor),
                                                                new VariableTensor("argument"), (a, b) -> a * b),
                                                       Reduce.Aggregator.sum).toPrimitive();
        MapEvaluationContext context = new MapEvaluationContext();

        for (Tensor tensorElement : tensors) { // tensors.size() = 1 for larger tensor
            context.put("argument", tensorElement);
            // System.out.println("Dot product of " + tensor + " and " + tensorElement + ": " + dotProductFunction.evaluate(context).asDouble());
            sum += dotProductFunction.evaluate(context).asDouble();
        }
        return sum;
    }

    private Tensor vector(TensorType.Dimension.Type dimensionType) {
        return vectors(dimensionType, 1).get(0);
    }
    
    /** Create a list of vectors having a single dimension x */
    private List<Tensor> vectors(TensorType.Dimension.Type dimensionType, int vectorCount) {
        int vectorSize = 3;
        List<Tensor> tensors = new ArrayList<>();
        TensorType type = new TensorType.Builder().dimension("x", dimensionType).build();
        for (int i = 0; i < vectorCount; i++) {
            Tensor.Builder builder = Tensor.Builder.of(type);
            for (int j = 0; j < vectorSize; j++) {
                builder.cell().label("x", String.valueOf(j)).value((i+1)*(j+1));
            }
            tensors.add(builder.build());
        }
        return tensors;
    }

    /** 
     * Create a matrix of vectors (in dimension i) where each vector has the dimension x.
     * This matrix contains the same vectors as returned by createVectors, in a single list element for convenience.
     */
    private List<Tensor> matrix(TensorType.Dimension.Type dimensionType, int vectorCount) {
        int vectorSize = 3;
        TensorType type = new TensorType.Builder().dimension("i", dimensionType).dimension("x", dimensionType).build();
        Tensor.Builder builder = Tensor.Builder.of(type);
        for (int i = 0; i < vectorCount; i++) {
            for (int j = 0; j < vectorSize; j++) {
                builder.cell()
                        .label("i", String.valueOf(i))
                        .label("x", String.valueOf(j))
                        .value((i+1)*(j+1));
            }
        }
        return Collections.singletonList(builder.build());
    }

}
