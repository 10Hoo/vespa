package com.yahoo.tensor.functions;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.yahoo.tensor.MapTensor;
import com.yahoo.tensor.Tensor;
import com.yahoo.tensor.TensorAddress;
import com.yahoo.tensor.TensorType;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * The <i>rename</i> tensor function returns a tensor where some dimensions are assigned new names.
 * 
 * @author bratseth
 */
public class Rename extends PrimitiveTensorFunction {

    private final TensorFunction argument;
    private final List<String> fromDimensions;
    private final List<String> toDimensions;

    public Rename(TensorFunction argument, List<String> fromDimensions, List<String> toDimensions) {
        Objects.requireNonNull(argument, "The argument tensor cannot be null");
        Objects.requireNonNull(fromDimensions, "The 'from' dimensions cannot be null");
        Objects.requireNonNull(toDimensions, "The 'to' dimensions cannot be null");
        if (fromDimensions.size() < 1)
            throw new IllegalArgumentException("from dimensions is empty, must rename at least one dimension");
        if (fromDimensions.size() != toDimensions.size())
            throw new IllegalArgumentException("Rename from and to dimensions must be equal, was " +
                                               fromDimensions.size() + " and " + toDimensions.size());
        this.argument = argument;
        this.fromDimensions = ImmutableList.copyOf(fromDimensions);
        this.toDimensions = ImmutableList.copyOf(toDimensions);
    }
    
    @Override
    public List<TensorFunction> functionArguments() { return Collections.singletonList(argument); }

    @Override
    public TensorFunction replaceArguments(List<TensorFunction> arguments) {
        if ( arguments.size() != 1)
            throw new IllegalArgumentException("Rename must have 1 argument, got " + arguments.size());
        return new Rename(arguments.get(0), fromDimensions, toDimensions);
    }

    @Override
    public PrimitiveTensorFunction toPrimitive() { return this; }

    @Override
    public Tensor evaluate(EvaluationContext context) {
        Tensor tensor = argument.evaluate(context);

        Map<String, String> fromToMap = fromToMap();
        TensorType renamedType = rename(tensor.type(), fromToMap);
        
        // an array which lists the index of each label in the renamed type
        int[] toIndexes = new int[tensor.type().dimensions().size()];
        for (int i = 0; i < tensor.type().dimensions().size(); i++) {
            String dimensionName = tensor.type().dimensions().get(i).name();
            String newDimensionName = fromToMap.getOrDefault(dimensionName, dimensionName);
            toIndexes[i] = renamedType.indexOfDimension(newDimensionName).get();
        }
            
        ImmutableMap.Builder<TensorAddress, Double> renamedCells = new ImmutableMap.Builder<>();
        for (Map.Entry<TensorAddress, Double> cell : tensor.cells().entrySet()) {
            TensorAddress renamedAddress = rename(cell.getKey(), toIndexes);
            renamedCells.put(renamedAddress, cell.getValue());
        }
        return new MapTensor(renamedType, renamedCells.build());
    }

    private TensorType rename(TensorType type, Map<String, String> fromToMap) {
        TensorType.Builder builder = new TensorType.Builder();
        for (TensorType.Dimension dimension : type.dimensions())
            builder.dimension(dimension.withName(fromToMap.getOrDefault(dimension.name(), dimension.name())));
        return builder.build();
    }
    
    private TensorAddress rename(TensorAddress address, int[] toIndexes) {
        String[] reorderedLabels = new String[toIndexes.length];
        for (int i = 0; i < toIndexes.length; i++)
            reorderedLabels[toIndexes[i]] = address.labels().get(i);
        return new TensorAddress(reorderedLabels);
    }

    @Override
    public String toString(ToStringContext context) { 
        return "rename(" + argument.toString(context) + ", " + 
                       toVectorString(fromDimensions) + ", " + toVectorString(toDimensions) + ")";
    }
    
    private Map<String, String> fromToMap() {
        Map<String, String> map = new HashMap<>();
        for (int i = 0; i < fromDimensions.size(); i++)
            map.put(fromDimensions.get(i), toDimensions.get(i));
        return map;
    }
    
    private String toVectorString(List<String> elements) {
        if (elements.size() == 1)
            return elements.get(0);
        StringBuilder b = new StringBuilder("[");
        for (String element : elements)
            b.append(element).append(", ");
        b.setLength(b.length() - 2);
        return b.toString();
    }

}
