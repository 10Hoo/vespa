// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vector>

namespace search {
namespace fef {

/**
 * Represents the type of a parameter.
 */
struct ParameterType {
    enum Enum {
        NONE,
        FIELD,           // for match information in a field
        INDEX_FIELD,     // for match information in an index field
        ATTRIBUTE_FIELD, // for match information in an attribute field
        ATTRIBUTE,       // for accessing an attribute vector
        FEATURE,         // a complete rank feature name
        NUMBER,
        STRING
    };
};

/**
 * Represents the accepted collection types for a field parameter.
 **/
struct ParameterCollection {
    enum Enum {
        NONE,
        SINGLE,      // single value
        ARRAY,       // unweighted multi-value
        WEIGHTEDSET, // weighted multi-value
        ANY          // any collection type
    };
};

/**
 * The description of a single parameter within a single
 * ParameterDescription object.
 **/
struct ParamDescItem {
    ParameterType::Enum type;
    ParameterCollection::Enum collection;
    ParamDescItem(ParameterType::Enum t,
                  ParameterCollection::Enum c)
        : type(t), collection(c) {}
};

/**
 * This class represents a set of parameter descriptions that each indicate what are a valid input parameter list for a Blueprint.
 * During setup of a Blueprint the descriptions can be used to validate the input parameter
 * list for that Blueprint. The parameters are valid if one of the descriptions match the actual parameter list.
 */
class ParameterDescriptions {
public:
    /**
     * This class represents a single parameter description, consisting of a list of parameter types.
     * This list of types must match the actual parameter list.
     */
    class Description {
    private:
        size_t _tag;
        std::vector<ParamDescItem> _params;
        size_t _repeat;
    public:
        /**
         * Creates a new object with the given tag.
         */
        Description(size_t tag);
        Description & addParameter(const ParamDescItem &param) {
            _params.push_back(param);
            return *this;
        }

        /**
         * Sets the repeat number.
         * This indicates that the last <repeat> parameter types can occur 0-n times.
         * The repeat should only be set after all parameter types are added.
         */
        Description & setRepeat(size_t repeat) {
            _repeat = repeat;
            return *this;
        }
        size_t getTag() const { return _tag; }
        const std::vector<ParamDescItem> & getParams() const { return _params; }
        /**
         * Returns the parameter type with the given index.
         * If this description has repeat the index can be out of bounds (the correct repeat parameter will be returned).
         */
        ParamDescItem getParam(size_t i) const;
        bool hasRepeat() const { return _repeat != 0; }
        size_t getRepeat() const { return _repeat; }
    };
    typedef std::vector<Description> DescriptionVector;

private:
    DescriptionVector _descriptions;
    size_t _nextTag;

    Description & getCurrent() { return _descriptions.back(); }
    void addParameter(const ParamDescItem &param) {
        assert(!_descriptions.empty());
        assert(!getCurrent().hasRepeat());
        getCurrent().addParameter(param);
    }
    void addParameter(ParameterType::Enum type, ParameterCollection::Enum collection) {
        addParameter(ParamDescItem(type, collection));
    }
    void addParameter(ParameterType::Enum type) {
        addParameter(type, ParameterCollection::ANY);
    }

public:
    /**
     * Creates a new object with no descriptions.
     */
    ParameterDescriptions();
    const DescriptionVector & getDescriptions() const { return _descriptions; }
    ParameterDescriptions & desc() {
        _descriptions.push_back(Description(_nextTag++));
        return *this;
    }
    /**
     * Starts a new description with the given tag.
     */
    ParameterDescriptions & desc(size_t tag) {
        _descriptions.push_back(Description(tag));
        _nextTag = tag + 1;
        return *this;
    }
    /**
     * Adds a field parameter to the current description.
     */
    ParameterDescriptions & field() {
        addParameter(ParameterType::FIELD);
        return *this;
    }
    /**
     * Adds an index field parameter to the current description.
     */
    ParameterDescriptions & indexField(ParameterCollection::Enum collection) {
        addParameter(ParameterType::INDEX_FIELD, collection);
        return *this;
    }
    /**
     * Adds an attribute field parameter to the current description.
     */
    ParameterDescriptions & attributeField(ParameterCollection::Enum collection) {
        addParameter(ParameterType::ATTRIBUTE_FIELD, collection);
        return *this;
    }
    /**
     * Adds an attribute parameter to the current description.
     */
    ParameterDescriptions & attribute(ParameterCollection::Enum collection) {
        addParameter(ParameterType::ATTRIBUTE, collection);
        return *this;
    }
    /**
     * Adds a feature parameter to the current description.
     */
    ParameterDescriptions & feature() {
        addParameter(ParameterType::FEATURE);
        return *this;
    }
    /**
     * Adds a number parameter to the current description.
     */
    ParameterDescriptions & number() {
        addParameter(ParameterType::NUMBER);
        return *this;
    }
    /**
     * Adds a string parameter to the current description.
     */
    ParameterDescriptions & string() {
        addParameter(ParameterType::STRING);
        return *this;
    }
    /**
     * Sets the repeat number on the current description.
     */
    ParameterDescriptions & repeat(size_t n = 1) {
        assert(!_descriptions.empty());
        assert(getCurrent().getParams().size() >= n);
        getCurrent().setRepeat(n);
        return *this;
    }
};



} // namespace fef
} // namespace search

