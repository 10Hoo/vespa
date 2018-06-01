// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

package com.yahoo.searchlib.rankingexpression.integration.ml.importer.onnx;

import com.yahoo.searchlib.rankingexpression.evaluation.TensorValue;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.IntermediateGraph;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.OrderedTensorType;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.Argument;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.ConcatV2;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.Constant;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.Identity;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.IntermediateOperation;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.Join;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.Map;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.MatMul;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.NoOp;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.Reshape;
import com.yahoo.searchlib.rankingexpression.integration.ml.importer.operations.Shape;
import com.yahoo.tensor.functions.ScalarFunctions;
import onnx.Onnx;

import java.util.List;
import java.util.stream.Collectors;

public class GraphImporter {

    public static IntermediateOperation mapOperation(Onnx.NodeProto node,
                                                     List<IntermediateOperation> inputs,
                                                     IntermediateGraph graph) {
        String nodeName = node.getName();
        String modelName = graph.name();

        switch (node.getOpType().toLowerCase()) {
            case "add":         return new Join(modelName, nodeName, inputs, ScalarFunctions.add());
            case "acos":        return new Map(modelName, nodeName, inputs, ScalarFunctions.acos());
            case "concat":      return new ConcatV2(modelName, nodeName, inputs);
            case "div":         return new Join(modelName, nodeName, inputs, ScalarFunctions.divide());
            case "floor":       return new Map(modelName, nodeName, inputs, ScalarFunctions.floor());
            case "identity":    return new Identity(modelName, nodeName, inputs);
            case "reshape":     return new Reshape(modelName, nodeName, inputs);
            case "shape":       return new Shape(modelName, nodeName, inputs);
            case "matmul":      return new MatMul(modelName, nodeName, inputs);
            case "max":         return new Join(modelName, nodeName, inputs, ScalarFunctions.max());
            case "min":         return new Join(modelName, nodeName, inputs, ScalarFunctions.min());
            case "mul":         return new Join(modelName, nodeName, inputs, ScalarFunctions.multiply());
            case "sqrt":        return new Map(modelName, nodeName, inputs, ScalarFunctions.sqrt());
            case "sigmoid":     return new Map(modelName, nodeName, inputs, ScalarFunctions.sigmoid());
            case "sub":         return new Join(modelName, nodeName, inputs, ScalarFunctions.subtract());
            case "elu":         return new Map(modelName, nodeName, inputs, ScalarFunctions.elu());
            case "relu":        return new Map(modelName, nodeName, inputs, ScalarFunctions.relu());
            case "selu":        return new Map(modelName, nodeName, inputs, ScalarFunctions.selu());
        }

        IntermediateOperation op = new NoOp(modelName, node.getName(), inputs);
        op.warning("Operation '" + node.getOpType() + "' is currently not implemented");
        return op;
    }

    public static IntermediateGraph importGraph(String modelName, Onnx.ModelProto model) {
        Onnx.GraphProto onnxGraph = model.getGraph();

        IntermediateGraph intermediateGraph = new IntermediateGraph(modelName);
        importOperations(onnxGraph, intermediateGraph);
        verifyOutputTypes(onnxGraph, intermediateGraph);

        return intermediateGraph;
    }

    private static void importOperations(Onnx.GraphProto onnxGraph, IntermediateGraph intermediateGraph) {
        for (Onnx.ValueInfoProto valueInfo : onnxGraph.getOutputList()) {
            importOperation(valueInfo.getName(), onnxGraph, intermediateGraph);
        }
    }

    private static IntermediateOperation importOperation(String name,
                                                         Onnx.GraphProto onnxGraph,
                                                         IntermediateGraph intermediateGraph) {
        if (intermediateGraph.alreadyImported(name)) {
            return intermediateGraph.get(name);
        }
        IntermediateOperation operation;
        if (isArgumentTensor(name, onnxGraph)) {
            Onnx.ValueInfoProto valueInfoProto = getArgumentTensor(name, onnxGraph);
            if (valueInfoProto == null)
                throw new IllegalArgumentException("Could not find argument tensor: " + name);
            OrderedTensorType type = TypeConverter.fromOnnxType(valueInfoProto.getType());
            operation = new Argument(intermediateGraph.name(), valueInfoProto.getName(), type);

            intermediateGraph.inputs(intermediateGraph.defaultSignature())
                    .put(IntermediateOperation.namePartOf(name), operation.vespaName());

        } else if (isConstantTensor(name, onnxGraph)) {
            Onnx.TensorProto tensorProto = getConstantTensor(name, onnxGraph);
            OrderedTensorType defaultType = OrderedTensorType.fromDimensionList(tensorProto.getDimsList());
            operation = new Constant(intermediateGraph.name(), name, defaultType);
            operation.setConstantValueFunction(type -> new TensorValue(TensorConverter.toVespaTensor(tensorProto, type)));

        } else {
            Onnx.NodeProto node = getNodeFromGraph(name, onnxGraph);
            List<IntermediateOperation> inputs = importOperationInputs(node, onnxGraph, intermediateGraph);
            operation = mapOperation(node, inputs, intermediateGraph);

            if (isOutputNode(name, onnxGraph)) {
                intermediateGraph.outputs(intermediateGraph.defaultSignature())
                        .put(IntermediateOperation.namePartOf(name), operation.vespaName());
            }
        }
        intermediateGraph.put(operation.vespaName(), operation);

        return operation;
    }

    private static boolean isArgumentTensor(String name, Onnx.GraphProto graph) {
        Onnx.ValueInfoProto value = getArgumentTensor(name, graph);
        Onnx.TensorProto tensor = getConstantTensor(name, graph);
        return value != null && tensor == null;
    }

    private static boolean isConstantTensor(String name, Onnx.GraphProto graph) {
        Onnx.ValueInfoProto value = getArgumentTensor(name, graph);
        Onnx.TensorProto tensor = getConstantTensor(name, graph);
        return value != null && tensor != null;
    }

    private static Onnx.ValueInfoProto getArgumentTensor(String name, Onnx.GraphProto graph) {
        for (Onnx.ValueInfoProto valueInfo : graph.getInputList()) {
            if (valueInfo.getName().equals(name)) {
                return valueInfo;
            }
        }
        return null;
    }

    private static Onnx.TensorProto getConstantTensor(String name, Onnx.GraphProto graph) {
        for (Onnx.TensorProto tensorProto : graph.getInitializerList()) {
            if (tensorProto.getName().equals(name)) {
                return tensorProto;
            }
        }
        return null;
    }

    private static boolean isOutputNode(String name, Onnx.GraphProto graph) {
        return getOutputNode(name, graph) != null;
    }

    private static Onnx.ValueInfoProto getOutputNode(String name, Onnx.GraphProto graph) {
        for (Onnx.ValueInfoProto valueInfo : graph.getOutputList()) {
            if (valueInfo.getName().equals(name)) {
                return valueInfo;
            }
            String nodeName = IntermediateOperation.namePartOf(valueInfo.getName());
            if (nodeName.equals(name)) {
                return valueInfo;
            }
        }
        return null;
    }

    private static List<IntermediateOperation> importOperationInputs(Onnx.NodeProto node,
                                                                     Onnx.GraphProto onnxGraph,
                                                                     IntermediateGraph intermediateGraph) {
        return node.getInputList().stream()
                .map(nodeName -> importOperation(nodeName, onnxGraph, intermediateGraph))
                .collect(Collectors.toList());
    }

    private static void verifyOutputTypes(Onnx.GraphProto onnxGraph, IntermediateGraph intermediateGraph) {
        for (String outputName : intermediateGraph.outputs(intermediateGraph.defaultSignature()).values()) {
            IntermediateOperation operation = intermediateGraph.get(outputName);
            Onnx.ValueInfoProto onnxNode = getOutputNode(outputName, onnxGraph);
            OrderedTensorType type = operation.type().orElseThrow(
                        () -> new IllegalArgumentException("Output of '" + outputName + "' has no type."));
            TypeConverter.verifyType(onnxNode.getType(), type);
        }
    }

    private static Onnx.NodeProto getNodeFromGraph(String nodeName, Onnx.GraphProto graph) {
        boolean hasPortNumber = nodeName.contains(":");
        for (Onnx.NodeProto node : graph.getNodeList()) {
            if (hasPortNumber) {
                for (String outputName : node.getOutputList()) {
                    if (outputName.equals(nodeName)) {
                        return node;
                    }
                }
            } else if (node.getName().equals(nodeName)) {
                return node;
            }
        }
        throw new IllegalArgumentException("Node '" + nodeName + "' not found in ONNX graph");
    }
}
