package com.yahoo.searchlib.rankingexpression.integration.onnx;

import com.yahoo.searchlib.rankingexpression.RankingExpression;
import com.yahoo.searchlib.rankingexpression.evaluation.Context;
import com.yahoo.searchlib.rankingexpression.evaluation.MapContext;
import com.yahoo.searchlib.rankingexpression.evaluation.TensorValue;
import com.yahoo.searchlib.rankingexpression.integration.tensorflow.TensorFlowImporter;
import com.yahoo.searchlib.rankingexpression.integration.tensorflow.TensorFlowModel;
import com.yahoo.tensor.Tensor;
import com.yahoo.tensor.TensorType;
import org.junit.Test;
import org.tensorflow.SavedModelBundle;

import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

/**
 * @author lesters
 */
public class OnnxMnistSoftmaxImportTestCase {

    @Test
    public void testMnistSoftmaxImport() throws IOException {
        OnnxModel model = new OnnxImporter().importModel("src/test/files/integration/onnx/mnist_softmax/mnist_softmax.onnx", "add");

        // Check constants
        assertEquals(2, model.largeConstants().size());

        Tensor constant0 = model.largeConstants().get("Variable_0");
        assertNotNull(constant0);
        assertEquals(new TensorType.Builder().indexed("d2", 784).indexed("d1", 10).build(),
                constant0.type());
        assertEquals(7840, constant0.size());

        Tensor constant1 = model.largeConstants().get("Variable_1_0");
        assertNotNull(constant1);
        assertEquals(new TensorType.Builder().indexed("d1", 10).build(),
                constant1.type());
        assertEquals(10, constant1.size());

        // Check required macros (inputs)
        assertEquals(1, model.requiredMacros().size());
        assertTrue(model.requiredMacros().containsKey("Placeholder_0"));
        assertEquals(new TensorType.Builder().indexed("d0").indexed("d1", 784).build(),
                model.requiredMacros().get("Placeholder_0"));

        // Check outputs
        RankingExpression output = model.expressions().get("add");
        assertNotNull(output);
        assertEquals("add", output.getName());
        assertEquals("join(reduce(join(rename(Placeholder_0, (d0, d1), (d0, d2)), constant(Variable_0), f(a,b)(a * b)), sum, d2), constant(Variable_1_0), f(a,b)(a + b))",
                output.getRoot().toString());
    }

    @Test
    public void testComparisonBetweenOnnxAndTensorflow() {
        String tfModelPath = "src/test/files/integration/tensorflow/mnist_softmax/saved";
        String onnxModelPath = "src/test/files/integration/onnx/mnist_softmax/mnist_softmax.onnx";

        Tensor argument = placeholderArgument();
        Tensor tensorFlowResult = evaluateTensorFlowModel(tfModelPath, argument, "Placeholder", "add");
        Tensor onnxResult = evaluateOnnxModel(onnxModelPath, argument, "Placeholder_0", "add");

        assertEquals("Operation 'add' produces equal results", tensorFlowResult, onnxResult);
    }

    private Tensor evaluateTensorFlowModel(String path, Tensor argument, String input, String output) {
        SavedModelBundle tensorFlowModel = SavedModelBundle.load(path, "serve");
        TensorFlowModel model = new TensorFlowImporter().importModel("test", tensorFlowModel);
        return evaluateExpression(model.expressions().get(output), contextFrom(model), argument, input);
    }

    private Tensor evaluateOnnxModel(String path, Tensor argument, String input, String output) {
        OnnxModel model = new OnnxImporter().importModel(path, output);
        return evaluateExpression(model.expressions().get(output), contextFrom(model), argument, input);
    }

    private Tensor evaluateExpression(RankingExpression expression, Context context, Tensor argument, String input) {
        context.put(input, new TensorValue(argument));
        return expression.evaluate(context).asTensor();
    }

    private Context contextFrom(TensorFlowModel result) {
        MapContext context = new MapContext();
        result.largeConstants().forEach((name, tensor) -> context.put("constant(" + name + ")", new TensorValue(tensor)));
        result.smallConstants().forEach((name, tensor) -> context.put("constant(" + name + ")", new TensorValue(tensor)));
        return context;
    }

    private Context contextFrom(OnnxModel result) {
        MapContext context = new MapContext();
        result.largeConstants().forEach((name, tensor) -> context.put("constant(" + name + ")", new TensorValue(tensor)));
        result.smallConstants().forEach((name, tensor) -> context.put("constant(" + name + ")", new TensorValue(tensor)));
        return context;
    }

    private Tensor placeholderArgument() {
        Tensor.Builder b = Tensor.Builder.of(new TensorType.Builder().indexed("d0", 1).indexed("d1", 784).build());
        for (int d0 = 0; d0 < 1; d0++)
            for (int d1 = 0; d1 < 784; d1++)
                b.cell(d1 * 1.0 / 784, d0, d1);
        return b.build();
    }


}
