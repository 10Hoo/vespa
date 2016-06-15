// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.document.datatypes;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

/**
 * @author <a href="mailto:thomasg@yahoo-inc.com">Thomas Gundersen</a>
 */
public abstract class FieldPathIteratorHandler {

    public static class IndexValue {

        private int index;
        private FieldValue key;

        public int getIndex() {
            return index;
        }

        public FieldValue getKey() {
            return key;
        }

        public IndexValue() {
            index = -1;
            key = null;
        }

        public IndexValue(int index) {
            this.index = index;
            key = null;
        }

        public IndexValue(FieldValue key) {
            index = -1;
            this.key = key;
        }

        public String toString() {
            if (key != null) {
                return key.toString();
            } else {
                return "" + index;
            }
        }

        @Override
        public boolean equals(Object o) {
            IndexValue other = (IndexValue)o;

            if (key != null) {
                if (other.key != null && key.equals(other.key)) {
                    return true;
                }
                return false;
            }

            return index == other.index;
        }
    };

    public static class VariableMap extends TreeMap<String, IndexValue> {

        @Override
        public Object clone() {
            Map<String, IndexValue> map = new VariableMap();
            map.putAll(this);
            return map;
        }
    }

    private VariableMap variables = new VariableMap();

    public void onPrimitive(FieldValue fv) {

    }

    public boolean onComplex(FieldValue fv) {
        return true;
    }

    public ModificationStatus doModify(FieldValue fv) {
        return ModificationStatus.NOT_MODIFIED;
    }

    public enum ModificationStatus {
        MODIFIED, REMOVED, NOT_MODIFIED
    }

    public ModificationStatus modify(FieldValue fv) {
        return doModify(fv);
    }

    public boolean createMissingPath() {
        return false;
    }

    public VariableMap getVariables() {
        return variables;
    }
}
