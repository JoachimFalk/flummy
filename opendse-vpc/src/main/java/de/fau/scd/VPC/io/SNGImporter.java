// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 *
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 *
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */
package de.fau.scd.VPC.io;

import static de.fau.scd.VPC.helper.Common.classMap;
import static de.fau.scd.VPC.helper.Common.setAttributes;
import static de.fau.scd.VPC.helper.Common.toInstance;

import java.io.IOException;
import java.io.Serializable;
import java.lang.reflect.InvocationTargetException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import de.fau.scd.VPC.helper.Common;
import de.fau.scd.VPC.helper.UniquePool;
import de.fau.scd.VPC.io.SNGReader.SNGFormatErrorException;
import edu.uci.ics.jung.graph.util.EdgeType;
import net.sf.opendse.model.Application;
import net.sf.opendse.model.Attributes;
import net.sf.opendse.model.Communication;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.IAttributes;
import net.sf.opendse.model.Task;

public class SNGImporter {


    SNGImporter(SNGReader sngReader, UniquePool uniquePool) throws SNGFormatErrorException
    {
        this.uniquePool  = uniquePool;
        this.application = toApplication(sngReader.getDocumentElement());
    }

    public Application<Task, Dependency> getApplication() {
        return application;
    }

    static private class Port {
        public enum Direction { IN, OUT };

        public final String    name;
        public final Direction direction;

        Port(String n, Direction d) {
            name      = n;
            direction = d;
        }
    }

    static private class ActorType {
        public final String name;
        public final Map<String, Port> ports = new HashMap<String, Port>();

        ActorType(String n) {
            name = n;
        }
    }

    static private class ActorInstance {
        public final String    name;
        public final ActorType type;
        public final Task      task;
        public final Map<String, Port> unboundPorts = new HashMap<String, Port>();

        ActorInstance(String name, ActorType type, Task task) {
            this.name = name;
            this.type = type;
            this.task = task;
            unboundPorts.putAll(type.ports);
        }
    }

    /**
     * Convert a specification XML element to an application
     *
     * @param eNetworkGraph the networkGraph XML element
     * @return the application
     * @throws SNGFormatErrorException
     */
    protected Application<Task, Dependency> toApplication(org.w3c.dom.Element eNetworkGraph) throws SNGFormatErrorException {
        Application<Task, Dependency> application = new Application<Task, Dependency>();

        final Map<String, ActorType>     actorTypes     = new HashMap<String, ActorType>();
        final Map<String, ActorInstance> actorInstances = new HashMap<String, ActorInstance>();

        for (org.w3c.dom.Element eActorType : SNGReader.childElements(eNetworkGraph, "actorType")) {
            final ActorType actorType = toActorType(eActorType);
            if (actorTypes.containsKey(actorType.name))
                throw new SNGFormatErrorException("Duplicate actor type \""+actorType.name+"\"!");
            actorTypes.put(actorType.name, actorType);
        }
        for (org.w3c.dom.Element eActorInstance : SNGReader.childElements(eNetworkGraph, "actorInstance")) {
            final ActorInstance actorInstance = toActorInstance(eActorInstance, actorTypes);
            if (actorInstances.containsKey(actorInstance.name))
                throw new SNGFormatErrorException("Duplicate actor instance \""+actorInstance.name+"\"!");
            actorInstances.put(actorInstance.name, actorInstance);
            application.addVertex(actorInstance.task);
        }
        for (org.w3c.dom.Element eFifo : SNGReader.childElements(eNetworkGraph, "fifo")) {
            int size    = Integer.valueOf(eFifo.getAttribute("size"));
            int initial = Integer.valueOf(eFifo.getAttribute("initial"));

            final org.w3c.dom.Element eSource = SNGReader.childElement(eFifo, "source");
            final String sourceActor = eSource.getAttribute("actor");
            final String sourcePort  = eSource.getAttribute("port");
            final ActorInstance sourceActorInstance = actorInstances.get(sourceActor);
            if (sourceActorInstance == null)
                throw new SNGFormatErrorException("Unknown source actor instance \""+sourceActor+"\"!");

            final Communication message = new Communication(sourceActor+"."+sourcePort);
            {
                Dependency dependency = new Dependency(uniquePool.createUniqeName());
                application.addEdge(dependency, sourceActorInstance.task, message, EdgeType.DIRECTED);
            }
            final org.w3c.dom.Element eTarget = SNGReader.childElement(eFifo, "target");
            final String targetActor = eTarget.getAttribute("actor");
            final String targetPort  = eTarget.getAttribute("port");
            final ActorInstance targetActorInstance = actorInstances.get(targetActor);
            if (targetActorInstance == null)
                throw new SNGFormatErrorException("Unknown target actor instance \""+targetActorInstance+"\"!");
            {
                Dependency dependency = new Dependency(uniquePool.createUniqeName());
                application.addEdge(dependency, message, targetActorInstance.task, EdgeType.DIRECTED);
            }
        }
        return application;
    }

    /**
     * Convert a actorType XML element to an ActorType
     *
     * @param eActorType the actorType XML element
     * @return an ActorType
     * @throws SNGFormatErrorException
     */
    protected ActorType toActorType(org.w3c.dom.Element eActorType) throws SNGFormatErrorException {
        final String    actorTypeName = eActorType.getAttribute("name");
        final ActorType actorType     = new ActorType(actorTypeName);

        for (org.w3c.dom.Element ePort : SNGReader.childElements(eActorType, "actorType")) {
            String name = ePort.getAttribute("name");
            String type = ePort.getAttribute("type");
            Port.Direction d = Port.Direction.valueOf(type.toUpperCase());
            if (actorType.ports.containsKey(name))
                throw new SNGFormatErrorException("Duplicate actor port \""+name+"\" in actor type \""+actorTypeName+"\"!");
            actorType.ports.put(name, new Port(name, d));
        }
        return actorType;
    }

    /**
     * Convert a actorInstance XML element to an ActorInstance
     *
     * @param eActorInstance the actorInstance XML element
     * @return an ActorInstance
     * @throws SNGFormatErrorException
     */
    protected ActorInstance toActorInstance(org.w3c.dom.Element eActorInstance, Map<String, ActorType> actorTypes)
            throws SNGFormatErrorException
    {
        final String type = eActorInstance.getAttribute("type");
        final String name = eActorInstance.getAttribute("name");
        final ActorType actorType = actorTypes.get(type);
        if (actorType == null)
            throw new SNGFormatErrorException("Unknown actor type \""+type+"\" for actor instance \""+name+"\"!");
        final Task task = new Task(name);
        return new ActorInstance(name, actorType, task);
    }


    protected Class<?> getClass(String name) throws ClassNotFoundException {
        if (classMap.containsKey(name)) {
            return classMap.get(name);
        } else {
            return Class.forName(name);
        }
    }

    protected <E extends IAttributes> void addAttributes(org.w3c.dom.Element eElement, E element) {
        Attributes attributes = new Attributes();
        {
            org.w3c.dom.Attr name = eElement.getAttributeNode("name");
            if (name != null) {
                attributes.put("name", name.getValue());
            }
            org.w3c.dom.Attr id = eElement.getAttributeNode("id");
            if (id != null) {
                attributes.put("SNGID", id.getValue());
            }
        }
        for (org.w3c.dom.Element eAttribute : SNGReader.childElements(eElement, "opendseattr")) {
            org.w3c.dom.Attr name = eAttribute.getAttributeNode("name");
            if (name == null) {
                throw new IllegalArgumentException("no name given for attribute " + eAttribute);
            }
            Object value = toAttribute(eAttribute);
            attributes.put(name.getValue(), value);
        }
        if (!attributes.isEmpty())
            setAttributes(element, attributes);
    }

    protected Object toAttribute(org.w3c.dom.Element eAttribute) {
        org.w3c.dom.Attr type = eAttribute.getAttributeNode("type");
        org.w3c.dom.Attr javaType = eAttribute.getAttributeNode("javaType");
        org.w3c.dom.Attr value = eAttribute.getAttributeNode("value");

        if (type == null && javaType == null) {
            throw new IllegalArgumentException("no type given for attribute " + eAttribute);
        }
        Class<?> clazz = classMap.get(type.getValue());
        try {
            if (clazz == null)
                clazz = Class.forName(javaType.getValue());
        } catch (ClassNotFoundException e) {
            System.err.println("Class " + type.getValue() + " not found. Ignoring attribute value " + value.getValue());
            return null;
        }
        {
            if (Map.class.isAssignableFrom(clazz)) {
                return toAttributeMap(eAttribute, clazz);
            } else if (Collection.class.isAssignableFrom(clazz)) {
                return toAttributeCollection(eAttribute, clazz);
            } else {
                if (value == null) {
                    throw new IllegalArgumentException("no value given for attribute " + eAttribute);
                }
                return toAttributeObject(eAttribute, clazz, value.getValue());
            }
        }
    }

    /**
     * Constructs an attribute collection that contains all passed elements and
     * their corresponding class.
     *
     * @param eAttribute
     *            the attribute element to add the collection to
     * @param clazz
     *            the class of the objects that are to create
     * @return the constructed collection
     */
    @SuppressWarnings({ "rawtypes", "unchecked" })
    protected Object toAttributeMap(org.w3c.dom.Element eAttribute, Class<?> clazz) {
        Map map;
        try {
            map = (Map) clazz.getConstructor().newInstance();
        } catch (InstantiationException | IllegalAccessException | IllegalArgumentException | InvocationTargetException
                | NoSuchMethodException | SecurityException e) {
            throw new IllegalArgumentException("type value mismatch for attribute " + eAttribute);
        }
        for (org.w3c.dom.Element eNestedAttribute : SNGReader.childElements(eAttribute, "opendseattr")) {
            org.w3c.dom.Attr name = eNestedAttribute.getAttributeNode("name");
            if (name == null) {
                throw new IllegalArgumentException("no name given for attribute " + eAttribute);
            }
            Object value = toAttribute(eNestedAttribute);
            map.put(name.getValue(), value);
        }
        return map;
    }

    /**
     * Constructs an attribute collection that contains all passed elements and
     * their corresponding class.
     *
     * @param eAttribute
     *            the attribute element to add the collection to
     * @param clazz
     *            the class of the objects that are to create
     * @return the constructed collection
     */
    @SuppressWarnings({ "rawtypes", "unchecked" })
    protected Object toAttributeCollection(org.w3c.dom.Element eAttribute, Class<?> clazz) {
        Collection collectionAttribute;
        try {
            collectionAttribute = (Collection) clazz.getConstructor().newInstance();
        } catch (InstantiationException | IllegalAccessException | IllegalArgumentException | InvocationTargetException
                | NoSuchMethodException | SecurityException e) {
            throw new IllegalArgumentException("type value mismatch for attribute " + eAttribute);
        }
        for (org.w3c.dom.Element eNestedAttribute : SNGReader.childElements(eAttribute, "opendseattr")) {
            Object actualEntry = toAttribute(eNestedAttribute);
            collectionAttribute.add(actualEntry);
        }
        return collectionAttribute;
    }

    /**
     * Constructs an instance of the passed class that contains the passed
     * value.
     *
     * @param eAttribute
     *            the XML attribute to convert
     * @param clazz
     *            the class of the object that is to create
     * @param value
     *            the value of the object that is to create
     * @return the constructed object
     */
    protected Object toAttributeObject(org.w3c.dom.Element eAttribute, Class<?> clazz, String value) {
        Object object = null;

        if (clazz.equals(Boolean.class)) {
            if (value.equals("0")) {
                    value = "false";
            } else if (value.equals("1")) {
                    value = "true";
            }
        }
        try {
            object = toInstance(value, clazz);
        } catch (IllegalArgumentException | SecurityException | InstantiationException | IllegalAccessException
                | InvocationTargetException | NoSuchMethodException e) {
        }
        // "fallback procedure"
        if (object == null && clazz.equals(Serializable.class)) {
            try {
                object = Common.fromString(value);
            } catch (ClassNotFoundException | IOException e2) {
            }
        }
        if (object == null) {
            throw new IllegalArgumentException("type value mismatch for attribute " + eAttribute);
        }
        return object;
    }

    protected final UniquePool uniquePool;

    protected final Application<Task, Dependency> application;
}
