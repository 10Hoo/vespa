// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.model.builder.xml.dom;

import com.yahoo.collections.Tuple2;
import com.yahoo.config.ConfigurationRuntimeException;
import com.yahoo.config.codegen.CNode;
import com.yahoo.log.LogLevel;
import com.yahoo.yolean.Exceptions;
import com.yahoo.text.XML;
import com.yahoo.vespa.config.*;

import com.yahoo.vespa.config.util.ConfigUtils;
import org.w3c.dom.Element;

import java.util.*;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 * Builder that transforms xml config to a slime tree representation of the config. The root element of the xml config
 * must be named 'config' and have a 'name' attribute that matches the name of the {@link ConfigDefinition}. The values
 * are not validated against their types. That task is moved to the builders.
 *
 * @author lulf
 */
public class DomConfigPayloadBuilder {

    private static final Logger log = Logger.getLogger(DomConfigPayloadBuilder.class.getPackage().toString());

    private static final Pattern namePattern = ConfigDefinition.namePattern;
    private static final Pattern namespacePattern = ConfigDefinition.namespacePattern;

    /** The config definition, not null if not found */
    private final ConfigDefinition configDefinition;

    public DomConfigPayloadBuilder(ConfigDefinition configDefinition) {
        this.configDefinition = configDefinition;
    }

    /**
     * Builds a {@link ConfigPayloadBuilder} representing the input 'config' xml element.
     *
     * @param configE        The 'config' xml element
     * @return a new payload builder built from xml.
     */
    public ConfigPayloadBuilder build(Element configE) {
        parseConfigName(configE);

        ConfigPayloadBuilder payloadBuilder = new ConfigPayloadBuilder(configDefinition);
        for (Element child : XML.getChildren(configE)) {
            parseElement(child, payloadBuilder, null);
        }
        return payloadBuilder;
    }

    public static ConfigDefinitionKey parseConfigName(Element configE) {
        if (!configE.getNodeName().equals("config")) {
            throw new ConfigurationRuntimeException("The root element must be 'config', but was '" 
                                                    + configE.getNodeName() + "'.");
        }
        if (!configE.hasAttribute("name")) {
            throw new ConfigurationRuntimeException
                    ("The 'config' element must have a 'name' attribute that matches the name of the config definition.");
        }

        String xmlName = configE.getAttribute("name");
        final boolean xmlNamespaceAttributeExists = configE.hasAttribute("namespace");

        String xmlNamespace = null;
        // If name contains dots, rewrite to name and namespace
        if (xmlName.contains(".")) {
            Tuple2<String, String> t = ConfigUtils.getNameAndNamespaceFromString(xmlName);
            xmlName = t.first;
            xmlNamespace = t.second;
        } else {
            if (!xmlNamespaceAttributeExists) {
                log.log(LogLevel.WARNING, "No namespace in 'config name=" + xmlName + "', please specify one");
            }
        }

        if (!validName(xmlName)) {
            throw new ConfigurationRuntimeException("The config name '" + xmlName +
                    "' contains illegal characters. Only names with the pattern " + namePattern.toString() + " are legal.");
        }

        if (xmlNamespace == null) {
            xmlNamespace = configE.getAttribute("namespace");
            if (xmlNamespace == null || xmlNamespace.isEmpty()) {
                xmlNamespace = CNode.DEFAULT_NAMESPACE;
            }
        }
        if (!validNamespace(xmlNamespace)) {
            throw new ConfigurationRuntimeException("The config namespace '" + xmlNamespace +
                    "' contains illegal characters. Only namespaces with the pattern " + namespacePattern.toString() + " are legal.");
        }
        return new ConfigDefinitionKey(xmlName, xmlNamespace);
    }

    private static boolean validName(String name) {
        Matcher m = namePattern.matcher(name);
        return m.matches();
    }

    private static boolean validNamespace(String namespace) {
        Matcher m = namespacePattern.matcher(namespace);
        return m.matches();
    }

    private String extractName(Element element) {
        String initial = element.getNodeName();
        if (initial.indexOf('-') < 0) {
            return initial;
        }
        StringBuilder buf = new StringBuilder();
        boolean upcase = false;
        for (char ch : initial.toCharArray()) {
            if (ch == '-') {
                upcase = true;
            } else if (upcase && ch >= 'a' && ch <= 'z') {
                buf.append((char)('A' + ch - 'a'));
                upcase = false;
            } else {
                buf.append(ch);
                upcase = false;
            }
        }
        return buf.toString();
    }

    /**
     * Parse leaf value in an xml tree
     */
    private void parseLeaf(Element element, ConfigPayloadBuilder payloadBuilder, String parentName) {
        String name = extractName(element);
        String value = XML.getValue(element);
        if (value == null) {
            throw new ConfigurationRuntimeException("Element '" + name + "' must have either children or a value");
        }


        if (element.hasAttribute("index")) {
            // Check for legacy (pre Vespa 6) usage
            throw new IllegalArgumentException("The 'index' attribute on config labels is not supported - use <item>");
        } else if (element.hasAttribute("operation")) {
            // leaf array, currently the only supported operation is 'append'
            verifyLegalOperation(element);
            ConfigPayloadBuilder.Array a = payloadBuilder.getArray(name);
            a.append(value);
        } else if ("item".equals(name)) {
            if (parentName == null)
                throw new ConfigurationRuntimeException("<item> is a reserved keyword for array and map labels");
            if (element.hasAttribute("key")) {
                payloadBuilder.getMap(parentName).put(element.getAttribute("key"), value);
            } else {
                payloadBuilder.getArray(parentName).append(value);
            }
        } else {
            // leaf scalar, e.g. <intVal>3</intVal>
            payloadBuilder.setField(name, value);
        }
    }

    private void parseComplex(Element element, List<Element> children, ConfigPayloadBuilder payloadBuilder, String parentName) {
        String name = extractName(element);
         // Inner value
        if (element.hasAttribute("index")) {
            // Check for legacy (pre Vespa 6) usage
            throw new IllegalArgumentException("The 'index' attribute on config labels is not supported - use <item>");
        } else if (element.hasAttribute("operation")) {
            // inner array, currently the only supported operation is 'append'
            verifyLegalOperation(element);
            ConfigPayloadBuilder childPayloadBuilder = payloadBuilder.getArray(name).append();
            //Cursor array = node.setArray(name);
            for (Element child : children) {
                //Cursor struct = array.addObject();
                parseElement(child, childPayloadBuilder, name);
            }
        } else if ("item".equals(name)) {
            // Reserved item means array/map element as struct
            if (element.hasAttribute("key")) {
                ConfigPayloadBuilder childPayloadBuilder = payloadBuilder.getMap(parentName).get(element.getAttribute("key"));
                for (Element child : children) {
                    parseElement(child, childPayloadBuilder, parentName);
                }
            } else {
                ConfigPayloadBuilder.Array array = payloadBuilder.getArray(parentName);
                ConfigPayloadBuilder childPayloadBuilder = array.append();
                for (Element child : children) {
                    parseElement(child, childPayloadBuilder, parentName);
                }
            }
        } else {
            int numMatching = 0;
            for (Element child : children) {
                numMatching += ("item".equals(child.getTagName())) ? 1 : 0;
            }

            if (numMatching == 0) {
                // struct, e.g. <basicStruct>
                ConfigPayloadBuilder p = payloadBuilder.getObject(name);
                //Cursor struct = node.setObject(name);
                for (Element child : children)
                    parseElement(child, p, name);
            } else if (numMatching == children.size()) {
                // Array with <item labels>
                for (Element child : children) {
                    parseElement(child, payloadBuilder, name);
                }
            } else {
                throw new ConfigurationRuntimeException("<item> is a reserved keyword for array and map labels");
            }
        }
    }

    /**
     * Adds the values and children (recursively) in the given xml element to the given {@link ConfigPayloadBuilder}.
     * @param currElem  The element representing a config parameter.
     * @param payloadBuilder The builder to use when adding labels.
     */
    private void parseElement(Element currElem, ConfigPayloadBuilder payloadBuilder, String parentName) {
        List<Element> children = XML.getChildren(currElem);
        try {
            if (children.isEmpty()) {
                parseLeaf(currElem, payloadBuilder, parentName);
            } else {
                parseComplex(currElem, children, payloadBuilder, parentName);
            }
        } catch (Exception exception) {
            throw new ConfigurationRuntimeException("Error parsing element at " + XML.getNodePath(currElem, " > ") + ": " +
                    Exceptions.toMessageString(exception));
        }
    }

    private void verifyLegalOperation(Element currElem) {
        String operation = currElem.getAttribute("operation");
        if (! operation.equalsIgnoreCase("append"))
            throw new ConfigurationRuntimeException("The only supported array operation is 'append', got '"
                    + operation + "' at XML node '" + XML.getNodePath(currElem, " > ") + "'.");
    }

}
