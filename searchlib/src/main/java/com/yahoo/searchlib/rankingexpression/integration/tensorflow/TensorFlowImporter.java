package com.yahoo.searchlib.rankingexpression.integration.tensorflow;

import com.yahoo.searchlib.rankingexpression.RankingExpression;
import com.yahoo.searchlib.rankingexpression.rule.ExpressionNode;
import com.yahoo.searchlib.rankingexpression.rule.TensorFunctionNode;
import com.yahoo.tensor.TensorType;
import com.yahoo.tensor.functions.ScalarFunctions;
import com.yahoo.tensor.functions.TensorFunction;
import com.yahoo.yolean.Exceptions;
import org.tensorflow.SavedModelBundle;
import org.tensorflow.framework.GraphDef;
import org.tensorflow.framework.MetaGraphDef;
import org.tensorflow.framework.NodeDef;
import org.tensorflow.framework.SignatureDef;
import org.tensorflow.framework.TensorInfo;
import org.tensorflow.framework.TensorShapeProto;

import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Converts a saved TensorFlow model into a ranking expression and set of constants.
 *
 * @author bratseth
 */
public class TensorFlowImporter {

    private final OperationMapper operationMapper = new OperationMapper();

    /**
     * Imports a saved TensorFlow model from a directory.
     * The model should be saved as a pbtxt file.
     * The name of the model is taken as the db/pbtxt file name (not including the file ending).
     *
     * @param modelDir the directory containing the TensorFlow model files to import
     */
    public ImportResult importModel(String modelDir) {
        try (SavedModelBundle model = SavedModelBundle.load(modelDir, "serve")) {
            return importGraph(MetaGraphDef.parseFrom(model.metaGraphDef()), model);
        }
        catch (IOException e) {
            throw new IllegalArgumentException("Could not read TensorFlow model from directory '" + modelDir + "'", e);
        }
    }

    /** Imports a specific node as an putput given the name of that node. Useful for testing */
    public ImportResult importNode(String modelDir, String signatureName, String nodeName) {
        try (SavedModelBundle model = SavedModelBundle.load(modelDir, "serve")) {
            MetaGraphDef graph = MetaGraphDef.parseFrom(model.metaGraphDef());
            SignatureDef signatureDef = graph.getSignatureDefMap().get(signatureName);
            ImportResult result = new ImportResult();
            ImportResult.Signature signature = result.signature(signatureName);
            importInputs(signatureDef.getInputsMap(), signature);
            signature.output(nodeName,
                             new RankingExpression(nodeName, importNode(nodeName, graph.getGraphDef(), model, signature)));
            return result;
        }
        catch (IOException e) {
            throw new IllegalArgumentException("Could not read TensorFlow model from directory '" + modelDir + "'", e);
        }
    }

    private ImportResult importGraph(MetaGraphDef graph, SavedModelBundle model) {
        ImportResult result = new ImportResult();
        for (Map.Entry<String, SignatureDef> signatureEntry : graph.getSignatureDefMap().entrySet()) {
            ImportResult.Signature signature = result.signature(signatureEntry.getKey()); // Prefer key over "methodName"

            importInputs(signatureEntry.getValue().getInputsMap(), signature);
            for (Map.Entry<String, TensorInfo> output : signatureEntry.getValue().getOutputsMap().entrySet()) {
                String outputName = output.getKey();
                try {
                    ExpressionNode node = importOutput(output.getValue(), graph.getGraphDef(), model, signature);
                    signature.output(outputName, new RankingExpression(outputName, node));
                }
                catch (IllegalArgumentException e) {
                    result.warn("Skipping output '" + outputName + "' of " + signature +
                                ": " + Exceptions.toMessageString(e));
                }
            }
        }
        return result;
    }

    private void importInputs(Map<String, TensorInfo> inputInfoMap, ImportResult.Signature signature) {
        inputInfoMap.forEach((key, value) -> {
            String argumentName = nameOf(value.getName());
            TensorType argumentType = importTensorType(value.getTensorShape());
            // Arguments are (Placeholder) nodes, so not local to the signature:
            signature.owner().argument(argumentName, argumentType);
            signature.input(key, argumentName);
        });
    }

    private TensorType importTensorType(TensorShapeProto tensorShape) {
        TensorType.Builder b = new TensorType.Builder();
        for (TensorShapeProto.Dim dimension : tensorShape.getDimList()) {
            int dimensionSize = (int)dimension.getSize();
            if (dimensionSize >= 0)
                b.indexed("d" + b.rank(), dimensionSize);
            else
                b.indexed("d" + b.rank()); // unbound size
        }
        return b.build();
    }

    private ExpressionNode importOutput(TensorInfo output, GraphDef graph, SavedModelBundle model,
                                        ImportResult.Signature signature) {
        return importNode(nameOf(output.getName()), graph, model, signature);
    }

    private ExpressionNode importNode(String nodeName, GraphDef graph, SavedModelBundle model,
                                      ImportResult.Signature signature) {
        TensorFunction function = importNode(getNode(nodeName, graph), graph, model, signature).function();
        return new TensorFunctionNode(function); // wrap top level (only) as an expression
    }

    /** Recursively convert a graph of TensorFlow nodes into a Vespa tensor function expression tree */
    private TypedTensorFunction importNode(NodeDef tfNode, GraphDef graph, SavedModelBundle model,
                                           ImportResult.Signature signature) {
        return tensorFunctionOf(tfNode, graph, model, signature);
    }

    private TypedTensorFunction tensorFunctionOf(NodeDef tfNode, GraphDef graph, SavedModelBundle model,
                                                 ImportResult.Signature signature) {
        // Import arguments lazily below, as some nodes have arguments unused arguments leading to unsupported ops
        // TODO: Implement mapping of more functions from https://www.tensorflow.org/api_docs/python/
        switch (tfNode.getOp().toLowerCase()) {
            case "add" : case "add_n" : return operationMapper.join(importArguments(tfNode, graph, model, signature), ScalarFunctions.add());
            case "acos" : return operationMapper.map(importArguments(tfNode, graph, model, signature), ScalarFunctions.acos());
            case "placeholder" : return operationMapper.placeholder(tfNode, signature);
            case "identity" : return operationMapper.identity(tfNode, model, signature);
            case "matmul" : return operationMapper.matmul(importArguments(tfNode, graph, model, signature));
            case "softmax" : return operationMapper.softmax(importArguments(tfNode, graph, model, signature));
            default : throw new IllegalArgumentException("Conversion of TensorFlow operation '" + tfNode.getOp() + "' is not supported");
        }
    }

    private List<TypedTensorFunction> importArguments(NodeDef tfNode, GraphDef graph, SavedModelBundle model,
                                                      ImportResult.Signature signature) {
        return tfNode.getInputList().stream()
                                    .map(argNode -> importNode(getNode(nameOf(argNode), graph), graph, model, signature))
                                    .collect(Collectors.toList());
    }

    private NodeDef getNode(String name, GraphDef graph) {
        return graph.getNodeList().stream()
                                  .filter(node -> node.getName().equals(name))
                                  .findFirst()
                                  .orElseThrow(() -> new IllegalArgumentException("Could not find node '" + name + "'"));
    }

    /**
     * A method signature input and output has the form name:index.
     * This returns the name part without the index.
     */
    private String nameOf(String name) {
        return name.split(":")[0];
    }

}
