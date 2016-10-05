// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.application;

import com.yahoo.config.provision.Environment;
import com.yahoo.config.provision.RegionName;
import com.yahoo.data.access.simple.Value;
import com.yahoo.log.LogLevel;
import com.yahoo.text.XML;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import javax.xml.transform.TransformerException;
import java.util.*;
import java.util.logging.Logger;

/**
 * Handles overrides in a XML document according to the rules defined for multi environment application packages.
 *
 * @author lulf
 * @since 5.22
 */
class OverrideProcessor implements PreProcessor {

    private static final Logger log = Logger.getLogger(OverrideProcessor.class.getName());

    private final Environment environment;
    private final RegionName region;
    private static final String ATTR_ID  = "id";
    private static final String ATTR_ENV = "environment";
    private static final String ATTR_REG = "region";

    public OverrideProcessor(Environment environment, RegionName region) {
        this.environment = environment;
        this.region = region;
    }

    public Document process(Document input) throws TransformerException {
        if (1==1) return input;
        log.log(LogLevel.DEBUG, "Preprocessing overrides with " + environment + "." + region);
        Document ret = Xml.copyDocument(input);
        Element root = ret.getDocumentElement();
        applyOverrides(root, Context.empty());
        return ret;
    }

    private void applyOverrides(Element parent, Context context) {
        context = getParentContext(parent, context);

        Map<String, List<Element>> elementsByTagName = elementsByTagNameAndId(XML.getChildren(parent));

        retainOverriddenElements(elementsByTagName);

        // For each tag name, prune overrides
        for (Map.Entry<String, List<Element>> entry : elementsByTagName.entrySet()) {
            pruneOverrides(parent, entry.getValue(), context);
        }

        // Repeat for remaining children;
        for (Element child : XML.getChildren(parent)) {
            applyOverrides(child, context);
            // Remove attributes
            child.removeAttributeNS(XmlPreProcessor.deployNamespaceUri, ATTR_ENV);
            child.removeAttributeNS(XmlPreProcessor.deployNamespaceUri, ATTR_REG);
        }
    }

    private Context getParentContext(Element parent, Context context) {
        Optional<Environment> environment = context.environment;
        RegionName region = context.region;
        if ( ! environment.isPresent()) {
            environment = getEnvironment(parent);
        }
        if (region.isDefault()) {
            region = getRegion(parent);
        }
        return Context.create(environment, region);
    }

    /**
     * Prune overrides from parent according to deploy override rules.
     *
     * @param parent             Parent {@link Element} above children.
     * @param children           Children where one {@link Element} will remain as the overriding element
     * @param context            Current context with environment and region.
     */
    private void pruneOverrides(Element parent, List<Element> children, Context context) {
        checkConsistentInheritance(children, context);
        pruneNonMatchingEnvironmentsAndRegions(parent, children);
        retainMostSpecificEnvironmentAndRegion(parent, children, context);
    }

    /**
     * Ensures that environment and region does not change from something non-default to something else.
     */
    private void checkConsistentInheritance(List<Element> children, Context context) {
        for (Element child : children) {
            Optional<Environment> env = getEnvironment(child);
            RegionName reg = getRegion(child);
            if (env.isPresent() && context.environment.isPresent() && !env.equals(context.environment)) {
                throw new IllegalArgumentException("Environment in child (" + env.get() + ") differs from that inherited from parent (" + context.environment + ") at " + child);
            }
            if (!reg.isDefault() && !context.region.isDefault() && !reg.equals(context.region)) {
                throw new IllegalArgumentException("Region in child (" + reg + ") differs from that inherited from parent (" + context.region + ") at " + child);
            }
        }
    }

    /**
     * Prune elements that are not matching our environment and region
     */
    private void pruneNonMatchingEnvironmentsAndRegions(Element parent, List<Element> children) {
        Iterator<Element> elemIt = children.iterator();
        while (elemIt.hasNext()) {
            Element child = elemIt.next();
            Optional<Environment> env = getEnvironment(child);
            RegionName reg = getRegion(child);
            if ((env.isPresent() && !environment.equals(env.get())) || (!reg.isDefault() && !region.equals(reg))) {
                parent.removeChild(child);
                elemIt.remove();
            }
        }
    }

    /**
     * Find the most specific element and remove all others.
     */
    private void retainMostSpecificEnvironmentAndRegion(Element parent, List<Element> children, Context context) {
        Element bestMatchElement = null;
        int bestMatch = 0;
        for (Element child : children) {
            int currentMatch = 1;
            Optional<Environment> elementEnvironment = hasEnvironment(child) ? getEnvironment(child) : context.environment;
            RegionName elementRegion = hasRegion(child) ? getRegion(child) : context.region;
            if (elementEnvironment.isPresent() && elementEnvironment.get().equals(environment))
                currentMatch++;
            if ( ! elementRegion.isDefault() && elementRegion.equals(region))
                currentMatch++;

            if (currentMatch > bestMatch) {
                bestMatchElement = child;
                bestMatch = currentMatch;
            }
        }

        // Remove elements not specific
        for (Element child : children) {
            if (child != bestMatchElement) {
                parent.removeChild(child);
            }
        }
    }

    /**
     * Retains all elements where at least one element is overridden. Removes non-overridden elements from map.
     */
    private void retainOverriddenElements(Map<String, List<Element>> elementsByTagName) {
        Iterator<Map.Entry<String, List<Element>>> it = elementsByTagName.entrySet().iterator();
        while (it.hasNext()) {
            List<Element> elements = it.next().getValue();
            boolean hasOverrides = false;
            for (Element element : elements) {
                if (hasEnvironment(element) || hasRegion(element)) {
                    hasOverrides = true;
                }
            }
            if (!hasOverrides) {
                it.remove();
            }
        }
    }

    private boolean hasRegion(Element element) {
        return element.hasAttributeNS(XmlPreProcessor.deployNamespaceUri, ATTR_REG);
    }

    private boolean hasEnvironment(Element element) {
        return element.hasAttributeNS(XmlPreProcessor.deployNamespaceUri, ATTR_ENV);
    }

    private Optional<Environment> getEnvironment(Element element) {
        String env = element.getAttributeNS(XmlPreProcessor.deployNamespaceUri, ATTR_ENV);
        if (env == null || env.isEmpty()) {
            return Optional.empty();
        }
        return Optional.of(Environment.from(env));
    }

    private RegionName getRegion(Element element) {
        String reg = element.getAttributeNS(XmlPreProcessor.deployNamespaceUri, ATTR_REG);
        if (reg == null || reg.isEmpty()) {
            return RegionName.defaultName();
        }
        return RegionName.from(reg);
    }

    private Map<String, List<Element>> elementsByTagNameAndId(List<Element> children) {
        Map<String, List<Element>> elementsByTagName = new LinkedHashMap<>();
        // Index by tag name
        for (Element child : children) {
            String key = child.getTagName();
            if (child.hasAttribute(ATTR_ID)) {
                key += child.getAttribute(ATTR_ID);
            }
            if (!elementsByTagName.containsKey(key)) {
                elementsByTagName.put(key, new ArrayList<>());
            }
            elementsByTagName.get(key).add(child);
        }
        return elementsByTagName;
    }

    /**
     * Represents environment and region in a given context.
     */
    private static final class Context {

        final Optional<Environment> environment;

        final RegionName region;

        private Context(Optional<Environment> environment, RegionName region) {
            this.environment = environment;
            this.region = region;
        }

        static Context empty() {
            return new Context(Optional.empty(), RegionName.defaultName());
        }

        public static Context create(Optional<Environment> environment, RegionName region) {
            return new Context(environment, region);
        }

    }

}
