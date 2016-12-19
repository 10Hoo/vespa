// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.tensor;

import com.google.common.annotations.Beta;
import com.google.common.collect.ImmutableMap;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.Optional;
import java.util.Set;

/**
 * An indexed (dense) tensor backed by a double array.
 *
 * @author bratseth
 */
@Beta
public class IndexedTensor implements Tensor {

    /** The prescribed and possibly abstract type this is an instance of */
    private final TensorType type;
    
    /** The sizes of the dimensions of this in the order of the dimensions of the type */
    private final int[] dimensionSizes;
    
    private final double[] values;
    
    private IndexedTensor(TensorType type, int[] dimensionSizes, double[] values) {
        this.type = type;
        this.dimensionSizes = dimensionSizes;
        this.values = values;
    }

    @Override
    public int size() {
        return values.length;
    }

    /**
     * Returns an iterator over the cells of this. 
     * Cells are returned in order of increasing indexes in each dimension, increasing 
     * indexes of later dimensions in the dimension type before earlier.
     */
    @Override
    public Iterator<Map.Entry<TensorAddress, Double>> cellIterator() {
        return new CellIterator();
    }

    /**
     * Returns an iterator over the values of this.
     * Values are returned in order of increasing indexes in each dimension, increasing 
     * indexes of later dimensions in the dimension type before earlier.
     */
    @Override
    public Iterator<Double> valueIterator() {
        return new ValueIterator();
    }

    /**
     * Returns an iterator over value iterators where the outer iterator is over each unique value of the dimensions
     * given and the inner iterator is over each unique value of the rest of the dimensions, in the same order as
     * other iterator.
     * 
     * @param dimensions the names of the dimensions of the superspace
     * @param dimensionSizes the size of each dimension in the space we are returning values for, containing
     *                       one value per dimension of this tensor (in order). Each size may be the same or smaller
     *                       than the corresponding size of this tensor
     */
    public Iterator<SubspaceIterator> subspaceIterator(Set<String> dimensions, int[] dimensionSizes) {
        return new SuperspaceIterator(dimensions, dimensionSizes);
    }

    /** 
     * Returns the value at the given indexes
     * 
     * @param indexes the indexes into the dimensions of this. Must be one number per dimension of this
     * @throws IndexOutOfBoundsException if any of the indexes are out of bound or a wrong number of indexes are given
     */
    public double get(int ... indexes) {
        if (values.length == 0) return Double.NaN;
        return values[toValueIndex(indexes, dimensionSizes)];
    }

    /** Returns the value at this address, or NaN if there is no value at this address */
    @Override
    public double get(TensorAddress address) {
        // optimize for fast lookup within bounds
        try {
            return values[toValueIndex(address, dimensionSizes)];
        }
        catch (IndexOutOfBoundsException e) {
            return Double.NaN;
        }
    }

    /** Returns the value at these indexes */
    private double get(Indexes indexes) {
        return values[toValueIndex(indexes.indexesForReading(), dimensionSizes)];
    }

    // TODO: Change to get
    private static int toValueIndex(int[] indexes, int[] dimensionSizes) {
        if (indexes.length == 1) return indexes[0]; // for speed
        if (indexes.length == 0) return 0; // for speed

        int valueIndex = 0;
        for (int i = 0; i < indexes.length; i++)
            valueIndex += productOfDimensionsAfter(i, dimensionSizes) * indexes[i];
        return valueIndex;
    }

    // TODO: Change to get
    private static int toValueIndex(TensorAddress address, int[] dimensionSizes) {
        if (address.labels().isEmpty()) return 0;

        int valueIndex = 0;
        for (int i = 0; i < address.labels().size(); i++)
            valueIndex += productOfDimensionsAfter(i, dimensionSizes) * Integer.parseInt(address.labels().get(i));
        return valueIndex;
    }

    private static int productOfDimensionsAfter(int afterIndex, int[] dimensionSizes) {
        int product = 1;
        for (int i = afterIndex + 1; i < dimensionSizes.length; i++)
            product *= dimensionSizes[i];
        return product;
    }

    @Override
    public TensorType type() { return type; }

    /**
     * Returns the length of this in the nth dimension
     *
     * @throws IndexOutOfBoundsException if the index is larger than the number of dimensions in this tensor minus one
     */
    public int size(int dimension) {
        return dimensionSizes[dimension];
    }

    @Override
    public Map<TensorAddress, Double> cells() {
        if (dimensionSizes.length == 0)
            return values.length == 0 ? Collections.emptyMap() : Collections.singletonMap(TensorAddress.empty, values[0]);
        
        ImmutableMap.Builder<TensorAddress, Double> builder = new ImmutableMap.Builder<>();
        Indexes indexes = new Indexes(dimensionSizes, values.length);
        for (int i = 0; i < values.length; i++) {
            builder.put(indexes.toAddress(), values[i]);
            if (i < values.length -1)
                indexes.next();
        }
        return builder.build();
    }
    
    @Override
    public int hashCode() { return Arrays.hashCode(values); }

    @Override
    public String toString() { return Tensor.toStandardString(this); }

    @Override
    public boolean equals(Object o) {
        if ( ! (o instanceof Tensor)) return false;
        return Tensor.equals(this, (Tensor)o);
    }

    public abstract static class Builder implements Tensor.Builder {
        
        final TensorType type;
        
        private Builder(TensorType type) {
            this.type = type;
        }

        public static Builder of(TensorType type) {
            if (type.dimensions().stream().allMatch(d -> d instanceof TensorType.IndexedBoundDimension))
                return new BoundBuilder(type);
            else
                return new UnboundBuilder(type);
        }

        /** 
         * Create a builder with dimension size information for this instance. Must be one size entry per dimension,
         * and, agree with the type size information when specified in the type.
         * If sizes are completely specified in the type this size information is redundant.
         */
        public static Builder of(TensorType type, int[] dimensionSizes) {
            // validate
            if (dimensionSizes.length != type.dimensions().size())
                throw new IllegalArgumentException(dimensionSizes.length + " is the wrong number of dimension sizes " + 
                                                   " for " + type);
            for (int i = 0; i < dimensionSizes.length; i++ ) {
                Optional<Integer> size = type.dimensions().get(i).size();
                if (size.isPresent() && size.get() < dimensionSizes[i])
                    throw new IllegalArgumentException("Size of " + type.dimensions() + " is " + dimensionSizes[i] +
                                                       " but cannot be larger than " + size.get());
            }
            
            return new BoundBuilder(type, dimensionSizes);
        }

        public abstract Builder cell(double value, int ... indexes);
        
        protected double[] arrayFor(int[] dimensionSizes) {
            int productSize = 1;
            for (int dimensionSize : dimensionSizes)
                productSize *= dimensionSize;
            return new double[productSize];
        }

        @Override
        public TensorType type() { return type; }

        @Override
        public abstract IndexedTensor build();

    }
    
    /** A bound builder can create the double array directly */
    private static class BoundBuilder extends Builder {

        private int[] dimensionSizes;
        private double[] values;

        private BoundBuilder(TensorType type) {
            this(type, dimensionSizesOf(type));
        }

        private BoundBuilder(TensorType type, int[] dimensionSizes) {
            super(type);
            if ( dimensionSizes.length != type.dimensions().size())
                throw new IllegalArgumentException("Must have a dimension size entry for each dimension in " + type);
            this.dimensionSizes = dimensionSizes;
            values = arrayFor(dimensionSizes);
            Arrays.fill(values, Double.NaN);
        }
        
        private static int[] dimensionSizesOf(TensorType type) {
            int[] dimensionSizes = new int[type.dimensions().size()];
            for (int i = 0; i < type.dimensions().size(); i++)
                dimensionSizes[i] = type.dimensions().get(i).size().get();
            return dimensionSizes;
        }

        @Override
        public BoundBuilder cell(double value, int ... indexes) {
            values[toValueIndex(indexes, dimensionSizes)] = value;
            return this;
        }
        
        @Override
        public CellBuilder cell() {
            return new CellBuilder(type, this);
        }

        @Override
        public Builder cell(TensorAddress address, double value) {
            values[toValueIndex(address, dimensionSizes)] = value;
            return this;
        }

        @Override
        public IndexedTensor build() {
            // Note that we do not check for no NaN's here for performance reasons. 
            // NaN's don't get lost so leaving them in place should be quite benign 
            if (values.length == 1 && Double.isNaN(values[0]))
                values = new double[0];
            IndexedTensor tensor = new IndexedTensor(type, dimensionSizes, values);
            // prevent further modification
            dimensionSizes = null;
            values = null;
            return tensor;
        }

    }

    /**
     * A builder used when we don't know the size of the dimensions up front.
     * All values is all dimensions must be specified.
     */
    private static class UnboundBuilder extends Builder {

        /** List of List or Double */
        private List<Object> firstDimension = null;

        private UnboundBuilder(TensorType type) {
            super(type);
        }

        @Override
        public IndexedTensor build() {
            if (firstDimension == null) // empty
                return new IndexedTensor(type, new int[type.dimensions().size()], new double[] {});
            if (type.dimensions().isEmpty()) // single number
                return new IndexedTensor(type, new int[type.dimensions().size()], new double[] {(Double) firstDimension.get(0) });

            int[] dimensionSizes = findDimensionSizes(firstDimension);
            double[] values = arrayFor(dimensionSizes);
            fillValues(0, 0, firstDimension, dimensionSizes, values);
            return new IndexedTensor(type, dimensionSizes, values);
        }
        
        private int[] findDimensionSizes(List<Object> firstDimension) {
            List<Integer> dimensionSizeList = new ArrayList<>(type.dimensions().size());
            findDimensionSizes(0, dimensionSizeList, firstDimension);
            int[] dimensionSizes = new int[type.dimensions().size()]; // may be longer than the list but that's correct
            for (int i = 0; i < dimensionSizes.length; i++)
                dimensionSizes[i] = dimensionSizeList.get(i);
            return dimensionSizes;
        }

        @SuppressWarnings("unchecked")
        private void findDimensionSizes(int currentDimensionIndex, List<Integer> dimensionSizes, List<Object> currentDimension) {
            if (currentDimensionIndex == dimensionSizes.size())
                dimensionSizes.add(currentDimension.size());
            else if (dimensionSizes.get(currentDimensionIndex) != currentDimension.size())
                throw new IllegalArgumentException("Missing values in dimension " + 
                                                   type.dimensions().get(currentDimensionIndex) + " in " + type);
            
            for (Object value : currentDimension)
                if (value instanceof List)
                    findDimensionSizes(currentDimensionIndex + 1, dimensionSizes, (List<Object>)value);
        }

        @SuppressWarnings("unchecked")
        private void fillValues(int currentDimensionIndex, int offset, List<Object> currentDimension, 
                                int[] dimensionSizes, double[] values) {
            if (currentDimensionIndex < dimensionSizes.length - 1) { // recurse to next dimension
                for (int i = 0; i < currentDimension.size(); i++)
                    fillValues(currentDimensionIndex + 1,
                               offset + productOfDimensionsAfter(currentDimensionIndex, dimensionSizes) * i,
                               (List<Object>) currentDimension.get(i), dimensionSizes, values);
            } else { // last dimension - fill values
                for (int i = 0; i < currentDimension.size(); i++)
                    values[offset + i] = (double) currentDimension.get(i);
            }
        }

        @Override
        public CellBuilder cell() {
            return new CellBuilder(type, this);
        }

        @Override
        public Builder cell(TensorAddress address, double value) {
            int[] indexes = new int[address.labels().size()];
            for (int i = 0; i < address.labels().size(); i++) {
                try {
                    indexes[i] = Integer.parseInt(address.labels().get(i));
                } catch (NumberFormatException e) {
                    throw new IllegalArgumentException("Labels in an indexed tensor must be integers, not '" +
                                                       address.labels().get(i) + "'");
                }
            }
            cell(value, indexes);
            return this;
        }

        /**
         * Set a value using an index API. The number of indexes must be the same as the dimensions in the type of this.
         * Values can be written in any order but all values needed to make this dense must be provided
         * before building this.
         *
         * @return this for chaining
         */
        @SuppressWarnings("unchecked")
        @Override
        public Builder cell(double value, int... indexes) {
            if (indexes.length != type.dimensions().size())
                throw new IllegalArgumentException("Wrong number of indexes (" + indexes.length + ") for " + type);

            if (indexes.length == 0) {
                firstDimension = Collections.singletonList(value);
                return this;
            }

            if (firstDimension == null)
                firstDimension = new ArrayList<>();
            List<Object> currentValues = firstDimension;
            for (int dimensionIndex = 0; dimensionIndex < indexes.length; dimensionIndex++) {
                ensureCapacity(indexes[dimensionIndex], currentValues);
                if (dimensionIndex == indexes.length - 1) { // last dimension
                    currentValues.set(indexes[dimensionIndex], value);
                } else {
                    if (currentValues.get(indexes[dimensionIndex]) == null)
                        currentValues.set(indexes[dimensionIndex], new ArrayList<>());
                    currentValues = (List<Object>) currentValues.get(indexes[dimensionIndex]);
                }
            }
            return this;
        }

        /** Fill the given list with nulls if necessary to make sure it has a (possibly null) value at the given index */
        private void ensureCapacity(int index, List<Object> list) {
            while (list.size() <= index)
                list.add(list.size(), null);
        }

    }
    
    // TODO: Generalize to vector cell iterator?
    private final class CellIterator implements Iterator<Map.Entry<TensorAddress, Double>> {

        private int count = 0;
        private final Indexes indexes = new Indexes(dimensionSizes, values.length);

        @Override
        public boolean hasNext() {
            return count < indexes.size();
        }

        @Override
        public Map.Entry<TensorAddress, Double> next() {
            if ( ! hasNext()) throw new NoSuchElementException("No cell at " + indexes);
            
            Map.Entry<TensorAddress, Double> current = new Cell(indexes.toAddress(), values[count]); // TODO: Correct to get(indexes)
            
            count++;
            if (hasNext())
                indexes.next();

            return current;
        }
        
    }

    private class Cell implements Map.Entry<TensorAddress, Double> {

        private final TensorAddress address;
        private final Double value;

        private Cell(TensorAddress address, Double value) {
            this.address = address;
            this.value = value;
        }

        @Override
        public TensorAddress getKey() { return address; }

        @Override
        public Double getValue() { return value; }

        @Override
        public Double setValue(Double value) {
            throw new UnsupportedOperationException("A tensor cannot be modified");
        }

    }

    private final class ValueIterator implements Iterator<Double> {

        private int count = 0;

        @Override
        public boolean hasNext() {
            return count < values.length;
        }

        @Override
        public Double next() {
            try {
                return values[count++];
            }
            catch (IndexOutOfBoundsException e) {
                throw new NoSuchElementException("No element at position " + count);
            }
        }

    }
    
    private final class SuperspaceIterator implements Iterator<SubspaceIterator> {

        private final Indexes superindexes;

        /** true at indexes whose dimension subspaces iterate over */
        private final boolean[] subdimensionIndexes;
        
        /** 
         * The sizes of the space we'll return values of, one value for each dimension of this tensor,
         * which may be equal to or smaller than the sizes of this tensor 
         */
        private final int[] dimensionSizes;

        private int count = 0;
        
        private SuperspaceIterator(Set<String> superdimensionNames, int[] dimensionSizes) {
            this.dimensionSizes = dimensionSizes;
            
            boolean[] superdimensionIndexes = new boolean[dimensionSizes.length]; // for outer iterator
            subdimensionIndexes = new boolean [dimensionSizes.length]; // for inner iterator
            for (int i = 0; i < type.dimensions().size(); i++ ) {
                boolean superDimension = superdimensionNames.contains(type.dimensions().get(i).name());
                superdimensionIndexes[i] = superDimension;
                subdimensionIndexes[i] =  ! superDimension;
            }
            
            superindexes = new Indexes(dimensionSizes, superdimensionIndexes);
        }
        
        @Override
        public boolean hasNext() {
            return count < superindexes.size();
        }

        @Override
        public SubspaceIterator next() {
            if ( ! hasNext()) throw new NoSuchElementException("No cell at " + superindexes);
            SubspaceIterator subspace = new SubspaceIterator(subdimensionIndexes, superindexes.indexesCopy(), dimensionSizes);
            count++;
            if (hasNext())
                superindexes.next();
            return subspace;
        }

    }

    /**
     * An iterator over a subspace of this tensor. This is exposed to allow clients to query the size.
     */
    public final class SubspaceIterator implements Iterator<Map.Entry<TensorAddress, Double>> {

        private final Indexes indexes;
        private int count = 0;
        
        /** 
         * Creates a new subspace iterator
         * 
         * @param dimensionIndexes a boolean array with a true entry for dimensions we should iterate over and false
         *                         entries for all other dimensions
         * @param address the address of the first cell of this subspace. 
         */
        private SubspaceIterator(boolean[] dimensionIndexes, int[] address, int[] dimensionSizes) {
            this.indexes = new Indexes(dimensionSizes, dimensionIndexes, address);
        }
        
        /** Returns the total number of cells in this subspace */
        public int size() { 
            return indexes.size();
        }
        
        @Override
        public boolean hasNext() { return count < indexes.size(); }            

        @Override
        public Map.Entry<TensorAddress, Double> next() {
            if ( ! hasNext()) throw new NoSuchElementException("No cell at " + indexes);

            Map.Entry<TensorAddress, Double> current = new Cell(indexes.toAddress(), get(indexes));

            count++;
            if (hasNext())
                indexes.next();

            return current;
        }

    }
    
    /** An array of indexes into this tensor which are able to find the next index in the value order */
    private static class Indexes {
        
        private final int size;
        private final int[] indexes;
        
        private final int[] dimensionSizes;
        
        /** Only mutate (take next in) the dimension indexes which are true */
        private final boolean[] iteratingDimensions;

        private Indexes(int[] dimensionSizes, int size) {
            this(dimensionSizes, trueArray(dimensionSizes.length), size);
        }

        private Indexes(int[] dimensionSizes, boolean[] iteratingDimensions) {
            this(dimensionSizes, iteratingDimensions, computeSize(dimensionSizes, iteratingDimensions));
        }
        
        private Indexes(int[] dimensionSizes, boolean[] iteratingDimensionIndexes, int size) {
            this(dimensionSizes, iteratingDimensionIndexes, new int[dimensionSizes.length], size);
        }

        private Indexes(int[] dimensionSizes, boolean[] iteratingDimensions, int[] initialIndexes) {
            this(dimensionSizes, iteratingDimensions, initialIndexes, computeSize(dimensionSizes, iteratingDimensions));
        }

        private Indexes(int[] dimensionSizes, boolean[] iteratingDimensions, int[] initialIndexes, int size) {
            this.dimensionSizes = dimensionSizes;
            this.iteratingDimensions = iteratingDimensions;
            this.indexes = initialIndexes;
            this.size = size;
        }
        
        private static boolean[] trueArray(int size) {
            boolean[] array = new boolean[size];
            Arrays.fill(array, true);
            return array;
        }

        private static int computeSize(int[] dimensionSizes, boolean[] iteratingDimensions) {
            int size = 1;
            for (int dimensionIndex = 0; dimensionIndex < dimensionSizes.length; dimensionIndex++)
                if (iteratingDimensions[dimensionIndex])
                    size *= dimensionSizes[dimensionIndex];
            return size;
        }
        
        private static boolean anyTrue(boolean[] values) {
            for (boolean value : values)
                if (value) return true;
            return false;
        }

        /** Returns the number of values this will iterate over - i.e the product if the iterating dimension sizes */
        public int size() {
            return size;
        }

        private void next() {
            int currentDimension = indexes.length - 1;
            while ( ! iteratingDimensions[currentDimension] || 
                    indexes[currentDimension] + 1 == dimensionSizes[currentDimension]) {
                if ( iteratingDimensions[currentDimension])
                    indexes[currentDimension--] = 0; // carry over
                else // leave this dimension as-is
                    currentDimension--;
            }
            indexes[currentDimension]++;
        }

        /** Returns the address of the current position of these indexes */
        private TensorAddress toAddress() {
            // TODO: We may avoid the array copy by issuing a one-time-use address?
            return new TensorAddress(indexes);
        }
        
        private int[] indexesCopy() {
            return Arrays.copyOf(indexes, indexes.length);
        }

        /** Returns a copy of the indexes of this which must not be modified */
        private int[] indexesForReading() { return indexes; }

        @Override
        public String toString() {
            return "indexes " + Arrays.toString(indexes);
        }

    }

}
