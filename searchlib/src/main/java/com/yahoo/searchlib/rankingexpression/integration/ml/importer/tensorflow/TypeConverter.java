// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

package com.yahoo.searchlib.rankingexpression.integration.ml.importer.tensorflow;

import com.yahoo.searchlib.rankingexpression.integration.ml.importer.OrderedTensorType;
import com.yahoo.tensor.TensorType;
import org.tensorflow.framework.AttrValue;
import org.tensorflow.framework.NodeDef;
import org.tensorflow.framework.TensorShapeProto;

import java.util.List;

/**
 * Converts and verifies TensorFlow tensor types into Vespa tensor types.
 *
 * @author lesters
 */
public class TypeConverter {

    public static void verifyType(NodeDef node, OrderedTensorType type) {
        TensorShapeProto shape = tensorFlowShape(node);
        if (shape != null) {
            if (shape.getDimCount() != type.rank()) {
                throw new IllegalArgumentException("TensorFlow shape of '" + node.getName() + "' " +
                        "does not match Vespa shape");
            }
            for (int tensorFlowIndex = 0; tensorFlowIndex < type.dimensions().size(); ++tensorFlowIndex) {
                int vespaIndex = type.dimensionMap(tensorFlowIndex);
                TensorShapeProto.Dim tensorFlowDimension = shape.getDim(tensorFlowIndex);
                TensorType.Dimension vespaDimension = type.type().dimensions().get(vespaIndex);
                if (tensorFlowDimension.getSize() != vespaDimension.size().orElse(-1L)) {
                    throw new IllegalArgumentException("TensorFlow dimensions of '" + node.getName() + "' " +
                            "does not match Vespa dimensions");
                }
            }
        }
    }

    private static TensorShapeProto tensorFlowShape(NodeDef node) {
        AttrValue attrValueList = node.getAttrMap().get("_output_shapes");
        if (attrValueList == null) {
            throw new IllegalArgumentException("_output_shapes attribute of '" + node.getName() + "' " +
                    "does not exist");
        }
        if (attrValueList.getValueCase() != AttrValue.ValueCase.LIST) {
            throw new IllegalArgumentException("_output_shapes attribute of '" + node.getName() + "' " +
                    "is not of expected type");
        }
        List<TensorShapeProto> shapeList = attrValueList.getList().getShapeList();
        return shapeList.get(0); // support multiple outputs?
    }

    public static OrderedTensorType fromTensorFlowType(NodeDef node) {
        return fromTensorFlowType(node, "d");  // standard naming convention: d0, d1, ...
    }

    public static OrderedTensorType fromTensorFlowType(NodeDef node, String dimensionPrefix) {
        OrderedTensorType.Builder builder = new OrderedTensorType.Builder();
        TensorShapeProto shape = tensorFlowShape(node);
        for (int i = 0; i < shape.getDimCount(); ++ i) {
            String dimensionName = dimensionPrefix + i;
            TensorShapeProto.Dim tensorFlowDimension = shape.getDim(i);
            if (tensorFlowDimension.getSize() >= 0) {
                builder.add(TensorType.Dimension.indexed(dimensionName, tensorFlowDimension.getSize()));
            } else {
                builder.add(TensorType.Dimension.indexed(dimensionName));
            }
        }
        return builder.build();
    }

}
