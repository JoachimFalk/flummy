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
package de.fau.scd.SNG;

import static de.fau.scd.SNG.Common.classMap;
import static de.fau.scd.SNG.Common.setAttributes;
import static de.fau.scd.SNG.Common.toInstance;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.Serializable;
import java.io.StringReader;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.regex.MatchResult;
//import java.util.function.BiFunction;

import edu.uci.ics.jung.graph.util.EdgeType;
//import edu.uci.ics.jung.graph.util.Pair;
import net.sf.opendse.model.Application;
//import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Attributes;
import net.sf.opendse.model.Communication;
import net.sf.opendse.model.Dependency;
//import net.sf.opendse.model.Edge;
import net.sf.opendse.model.Element;
//import net.sf.opendse.model.Function;
import net.sf.opendse.model.IAttributes;
//import net.sf.opendse.model.Link;
//import net.sf.opendse.model.Mapping;
//import net.sf.opendse.model.Mappings;
//import net.sf.opendse.model.Node;
//import net.sf.opendse.model.Resource;
//import net.sf.opendse.model.Routings;
//import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.model.parameter.ParameterRange;
import net.sf.opendse.model.parameter.ParameterRangeDiscrete;
import net.sf.opendse.model.parameter.ParameterSelect;
import net.sf.opendse.model.parameter.ParameterUniqueID;
import net.sf.opendse.model.parameter.Parameters;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
//import javax.xml.parsers.SAXParser;
//import javax.xml.parsers.SAXParserFactory;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

//import org.w3c.dom.Attr;
//import org.w3c.dom.Document;
//import org.w3c.dom.Element;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

/**
 * The {@code SNGReader} reads a {@code Specification} from an
 * {@code InputStream} or file.
 *
 * @author Joachim Falk
 *
 */
public class SNGReader {

    static public class SNGFormatErrorException extends Exception {
        private static final long serialVersionUID = 3741956271022484454L;

        public SNGFormatErrorException(String message) {
            super(message);
        }
    }


    /**
     * Read specification from a file.
     *
     * @param filename
     *            The name of the file.
     * @throws SNGFormatErrorException
     */
    public SNGReader(String filename) throws FileNotFoundException, SNGFormatErrorException {
        this(new File(filename));
    }

    /**
     * Read specification from a file.
     *
     * @param file
     *            The file.
     * @throws SNGFormatErrorException
     */
    public SNGReader(File file) throws FileNotFoundException, SNGFormatErrorException {
        this(new StreamSource(new FileInputStream(file), file.toURI().toASCIIString()));
    }

    /**
     * Read specification from an input stream.
     *
     * @param in
     *            The input stream.
     * @throws SNGFormatErrorException
     */
    public SNGReader(InputStream in) throws SNGFormatErrorException {
        this(new StreamSource(in, "<input stream>"));
    }

    /**
     * Read specification from an input stream.
     *
     * @param in
     *            The input stream.
     * @throws SNGFormatErrorException
     */
    protected SNGReader(StreamSource in) throws SNGFormatErrorException {
        try {
            String sngXSDUrl = "sng.xsd";
            StreamSource sources[] = new StreamSource[] {
                    new StreamSource(new StringReader(SngXSD.text), sngXSDUrl)
                };
            SchemaFactory factory =
                SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
            factory.setErrorHandler(new DOMErrorHandler());
            Schema schema = factory.newSchema(sources);
            assert schema != null : "Can't parse '" + sngXSDUrl + "'!";
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setNamespaceAware(true);
            dbf.setValidating(false);
            dbf.setFeature("http://xml.org/sax/features/namespaces", true);
            dbf.setFeature("http://xml.org/sax/features/validation", false);
            dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-dtd-grammar", false);
            dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            dbf.setSchema(schema);
//          Property 'http://java.sun.com/xml/jaxp/properties/schemaLanguage' cannot be set
//          when a non-null Schema object has already been specified.
//          dbf.setAttribute(
//              "http://java.sun.com/xml/jaxp/properties/schemaLanguage",
//              XMLConstants.W3C_XML_SCHEMA_NS_URI);
            DocumentBuilder docBuilder = dbf.newDocumentBuilder();
            docBuilder.setErrorHandler(new DOMErrorHandler());
            org.w3c.dom.Document doc = docBuilder.parse(in.getInputStream(), in.getSystemId());
            doc.normalize();
            org.w3c.dom.Element eNetworkGraph = doc.getDocumentElement();
            //specification = toSpecification(eSpec);
            application = toApplication(eNetworkGraph);
        } catch (Exception ex) {
//          ex.printStackTrace(System.err);
            throw new SNGFormatErrorException(ex.getMessage());
        }
    }

    public Application<Task, Dependency> getApplication() {
        return application;
    }

    private static class DOMErrorHandler implements ErrorHandler {
        @Override
        public void warning(SAXParseException exception) throws SAXException {
            printError("Warning", exception);
        }

        @Override
        public void error(SAXParseException exception) throws SAXException {
            printError("Error", exception);
            throw exception;
        }

        @Override
        public void fatalError(SAXParseException exception) throws SAXException {
            printError("Fatal Error", exception);
            throw exception;
        }

        /** Prints the error message. */
        private void printError(String type, SAXParseException ex) {
            System.err.print("[");
            System.err.print(type);
            System.err.print("] ");
            String systemId = ex.getSystemId();
            if (systemId != null) {
//              int index = systemId.lastIndexOf('/');
//              if (index != -1)
//                  systemId = systemId.substring(index + 1);
                System.err.print(systemId);
            }
            System.err.print(':');
            System.err.print(ex.getLineNumber());
            System.err.print(':');
            System.err.print(ex.getColumnNumber());
            System.err.print(": ");
            System.err.print(ex.getMessage());
            System.err.println();
            System.err.flush();
        }
    }

    protected final Application<Task, Dependency> application;

    static protected class Port {
        public enum Direction { IN, OUT };

        public final String    name;
        public final Direction direction;

        Port(String n, Direction d) {
            name      = n;
            direction = d;
        }
    }

    static protected class ActorType {
        public final String name;
        public final Map<String, Port> ports = new HashMap<String, Port>();

        ActorType(String n) {
            name = n;
        }
    }

    static protected class ActorInstance {
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

        for (org.w3c.dom.Element eActorType : childElements(eNetworkGraph, "actorType")) {
            final ActorType actorType = toActorType(eActorType);
            if (actorTypes.containsKey(actorType.name))
                throw new SNGFormatErrorException("Duplicate actor type \""+actorType.name+"\"!");
            actorTypes.put(actorType.name, actorType);
        }
        for (org.w3c.dom.Element eActorInstance : childElements(eNetworkGraph, "actorInstance")) {
            final ActorInstance actorInstance = toActorInstance(eActorInstance, actorTypes);
            if (actorInstances.containsKey(actorInstance.name))
                throw new SNGFormatErrorException("Duplicate actor instance \""+actorInstance.name+"\"!");
            actorInstances.put(actorInstance.name, actorInstance);
            application.addVertex(actorInstance.task);
        }
        for (org.w3c.dom.Element eFifo : childElements(eNetworkGraph, "fifo")) {
            int size    = Integer.valueOf(eFifo.getAttribute("size"));
            int initial = Integer.valueOf(eFifo.getAttribute("initial"));

            final org.w3c.dom.Element eSource = childElement(eFifo, "source");
            final String sourceActor = eSource.getAttribute("actor");
            final String sourcePort  = eSource.getAttribute("port");
            final ActorInstance sourceActorInstance = actorInstances.get(sourceActor);
            if (sourceActorInstance == null)
                throw new SNGFormatErrorException("Unknown source actor instance \""+sourceActor+"\"!");

            final Communication message = new Communication(sourceActor+"."+sourcePort);
            {
                Dependency dependency = new Dependency(createUniqeName());
                application.addEdge(dependency, sourceActorInstance.task, message, EdgeType.DIRECTED);
            }
            final org.w3c.dom.Element eTarget = childElement(eFifo, "target");
            final String targetActor = eTarget.getAttribute("actor");
            final String targetPort  = eTarget.getAttribute("port");
            final ActorInstance targetActorInstance = actorInstances.get(targetActor);
            if (targetActorInstance == null)
                throw new SNGFormatErrorException("Unknown target actor instance \""+targetActorInstance+"\"!");
            {
                Dependency dependency = new Dependency(createUniqeName());
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

        for (org.w3c.dom.Element ePort : childElements(eActorType, "actorType")) {
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
/*
    protected static class ProblemGraph {
        public final List<Task> actors   = new ArrayList<Task>();
        public final List<Task> channels = new ArrayList<Task>();

        protected enum PortDirection {
            In, Out
        };

        static protected class ConnectionInfo {
            public ConnectionInfo() {
                actor = null;
                eActorPort = null;
                actorPortDirection = null;
                channels = new ArrayList<Task>();
            }

            /// This the actor from which the connection starts.
            public Task actor;
            /// This is the port of the actor that is used to start the
            /// connection.
            public org.w3c.dom.Element eActorPort;
            /// This is the direction of the data transfer from the point of
            /// view of the actor.
            public PortDirection actorPortDirection;
            /// This is the list of connected channel. This must be exactly one
            /// channel if the direction is In.
            public final List<Task> channels;
        }

        public final List<ConnectionInfo> connectionInfos = new ArrayList<ConnectionInfo>();
    }

    protected class ProblemGraphReader {

        protected final ProblemGraph problemGraph;
        protected final org.w3c.dom.Element eProblemGraph;

        protected final Map<Long, ProblemGraph.ConnectionInfo> expectedPorts = new HashMap<Long, ProblemGraph.ConnectionInfo>();

        ProblemGraphReader(ProblemGraph problemGraph,
                Map<Long, ProblemGraph.ConnectionInfo> problemGraphPorts, org.w3c.dom.Element eProblemGraph)
                throws SNGFormatErrorException {
            this.problemGraph = problemGraph;
            this.eProblemGraph = eProblemGraph;

            parsePortMappings(problemGraphPorts);
            parseEdges();
            parseProcesses();
        }

        protected void parsePortMappings(Map<Long, ProblemGraph.ConnectionInfo> problemGraphPorts)
                throws SNGFormatErrorException {
            for (org.w3c.dom.Element ePortMapping : childElements(eProblemGraph, "portmapping")) {
                long idTo = parseId(ePortMapping, "to");
                ProblemGraph.ConnectionInfo connectionInfo = problemGraphPorts.get(idTo);
                if (connectionInfo == null) {
                    throw new SNGFormatErrorException(
                            "To " + idTo + " of port mapping " + ePortMapping + " not found!");
                }
                long idFrom = parseId(ePortMapping, "from");
                if (expectedPorts.put(idFrom, connectionInfo) != null) {
                    throw new SNGFormatErrorException(
                            "From " + idFrom + " of port mapping " + ePortMapping + " is duplicate!");
                }
            }
        }

        protected void parseEdges() throws SNGFormatErrorException {
            for (org.w3c.dom.Element eEdge : childElements(eProblemGraph, "edge")) {
                long idSource = parseId(eEdge, "source");
                long idTarget = parseId(eEdge, "target");
                ProblemGraph.ConnectionInfo connectionInfo;
                ProblemGraph.ConnectionInfo sourceConnectionInfo = expectedPorts.get(idSource);
                ProblemGraph.ConnectionInfo targetConnectionInfo = expectedPorts.get(idTarget);
                if (sourceConnectionInfo != null && targetConnectionInfo != null) {
                    throw new SNGFormatErrorException("Edge " + eEdge + " from source id" + idSource + " to target id"
                            + idTarget + " is erroneous!");
                }
                if (sourceConnectionInfo != null) {
                    connectionInfo = sourceConnectionInfo;
                } else if (targetConnectionInfo != null) {
                    connectionInfo = targetConnectionInfo;
                } else {
                    connectionInfo = new ProblemGraph.ConnectionInfo();
                    problemGraph.connectionInfos.add(connectionInfo);
                }
                if (sourceConnectionInfo == null) {
                    expectedPorts.put(idSource, connectionInfo);
                }
                if (targetConnectionInfo == null) {
                    expectedPorts.put(idTarget, connectionInfo);
                }
            }
        }

        protected void parseProcesses() throws SNGFormatErrorException {
            for (org.w3c.dom.Element eProcess : childElements(eProblemGraph, "process")) {
                final Map<Long, ProblemGraph.ConnectionInfo> processPorts = new HashMap<Long, ProblemGraph.ConnectionInfo>();

                Task task = null;
                String taskType = eProcess.getAttribute("type");
                if (taskType.equals("actor")) {
                    task = createElement(eProcess, Task.class);
                    problemGraph.actors.add(task);
                } else if (taskType.equals("fifo")) {
                    task = createElement(eProcess, Task.class);
                    problemGraph.channels.add(task);
                }
                for (org.w3c.dom.Element ePort : childElements(eProcess, "port")) {
                    Long portId = parseId(ePort);

                    ProblemGraph.ConnectionInfo connectionInfo = expectedPorts.get(portId);
                    if (connectionInfo == null) {
                        throw new SNGFormatErrorException("Port " + ePort + " with id" + portId + " was not bound!");
                    }
                    processPorts.put(portId, connectionInfo);
                    String portDirectionStr = ePort.getAttribute("type");
                    ProblemGraph.PortDirection portDirection;
                    if (portDirectionStr.equals("in")) {
                        portDirection = ProblemGraph.PortDirection.In;
                    } else if (portDirectionStr.equals("out")) {
                        portDirection = ProblemGraph.PortDirection.Out;
                    } else {
                        throw new SNGFormatErrorException(
                                "Illegal port direction " + portDirectionStr + " for port " + ePort + "!");
                    }
                    if (taskType.equals("actor")) {
                        if (connectionInfo.actor != null) {
                            throw new SNGFormatErrorException("Two actor ports of the two actors " + task + " and "
                                    + connectionInfo.actor + " can't be connected together!");
                        }
                        connectionInfo.actor = task;
                        connectionInfo.eActorPort = ePort;
                        if (connectionInfo.actorPortDirection != null && connectionInfo.actorPortDirection != portDirection) {
                            throw new SNGFormatErrorException("Port directions for port id" + portId + " clash!");
                        }
                        connectionInfo.actorPortDirection = portDirection;
                    } else if (taskType.equals("fifo")) {
                        ProblemGraph.PortDirection actorPortDirection = portDirection == ProblemGraph.PortDirection.In
                                ? ProblemGraph.PortDirection.Out
                                : ProblemGraph.PortDirection.In;
                        connectionInfo.channels.add(task);
                        if (connectionInfo.actorPortDirection != null && connectionInfo.actorPortDirection != actorPortDirection) {
                            throw new SNGFormatErrorException("Port directions for port id" + portId + " clash!");
                        }
                        connectionInfo.actorPortDirection = actorPortDirection;
                    }
                }
                if (taskType.equals("graph")) {
                    new ProblemGraphReader(problemGraph, processPorts, childElement(eProcess, "problemgraph"));
                } else if (task == null) {
                    throw new SNGFormatErrorException("Unknown process type " + taskType + " for element " + eProcess);
                }
            }
        }
    }
  */
    protected Map<Long,   Element> knownElements = new HashMap<Long, Element>();

    protected <E extends Element> E createElement(org.w3c.dom.Element eElement, Class<E> type)
            throws SNGFormatErrorException {
        String name = createUniqeName(eElement);
        E retval;
        try {
            retval = type.getConstructor(String.class).newInstance(name);
        } catch (InstantiationException | IllegalAccessException | IllegalArgumentException |
                 InvocationTargetException | NoSuchMethodException | SecurityException e) {
            assert false : "This must never happen. Wrong type " + type + " given for createElement!";
            return null;
        }
        if (knownElements.put(parseId(eElement), retval) != null) {
            throw new SNGFormatErrorException(
                    "Non unique id " + eElement.getAttribute("id") + " for element " + eElement);
        }
        addAttributes(eElement, retval);
        return retval;
    }

    protected <E extends Element> E getElement(org.w3c.dom.Element eElement, String idRefAttr, Class<E> type)
            throws SNGFormatErrorException {
        Long idRef = parseId(eElement, idRefAttr);
        Element e = knownElements.get(idRef);
        if (e == null) {
            throw new SNGFormatErrorException("Element " + eElement + " has an unknown id reference id" + idRef);
        }
        if (!type.isInstance(e)) {
            throw new SNGFormatErrorException("Element " + eElement + " has an id reference id" + idRef
                    + " that referes to the wrong type " + e.getClass() + " while " + type + " was expected!");
        }
        return type.cast(e);
    }

    protected Map<String, Integer> knownNames    = new HashMap<String, Integer>();

    protected String createUniqeName(org.w3c.dom.Element eElement) {
        String name = eElement.getAttribute("name");
        // Fall back to id attribute.
        if (name == null)
            name = eElement.getAttribute("id");
        return createUniqeName(name, false);
    }

    protected String createUniqeName() {
        return createUniqeName("__anonymous", true);
    }

    protected String createUniqeName(String name, boolean numbered) {
        // Enforce unique names as names are used as IDs inside OpenDSE.
        Integer repeatEntry = knownNames.get(name);
        if (numbered && repeatEntry == null) {
            repeatEntry = 0;
        }
        if (repeatEntry != null) {
            String newName;
            int repeat = repeatEntry-1;
            do {
                repeat++;
                newName = name + "_" + Integer.toString(repeat);
            } while ((repeatEntry = knownNames.get(newName)) != null);
            knownNames.put(name, repeat);
            name = newName;
        }
        knownNames.put(name, 1);
        return name;
    }

    static protected
    long parseId(org.w3c.dom.Element eElement) throws SNGFormatErrorException {
        return parseId(eElement, "id");
    }

    static protected
    long parseId(org.w3c.dom.Element eElement, String attribute) throws SNGFormatErrorException {
        String id = eElement.getAttribute(attribute);
        if (id == null) {
            throw new SNGFormatErrorException("Missing attribute " + attribute + " for element " + eElement);
        }
        if (!id.startsWith("id")) {
            throw new SNGFormatErrorException("Wrong format for id " + id + " for element " + eElement);
        }
        for (int i = 2; i < id.length(); ++i) {
            if (id.charAt(i) < '0' || id.charAt(i) > '9') {
                throw new SNGFormatErrorException("Wrong format for id " + id + " for element " + eElement);
            }
        }
        try {
            return Long.parseLong(id.substring(2));
        } catch (NumberFormatException e) {
            throw new SNGFormatErrorException("Wrong format for id " + id + " for element " + eElement);
        }
    }

    @SuppressWarnings("unchecked")
    protected <C> Class<C> getClass(nu.xom.Element eElement) throws ClassNotFoundException {
        Class<C> type = null;
        if (eElement.getAttribute("class") != null) {
            type = (Class<C>) Class.forName(eElement.getAttributeValue("class"));
        } else {
            type = (Class<C>) classMap.get(eElement.getLocalName());
        }
        if (type == null) {
            throw new RuntimeException("Unknown node type for " + eElement);
        }
        return type;
    }

    protected Class<?> getClass(String name) throws ClassNotFoundException {
        if (classMap.containsKey(name)) {
            return classMap.get(name);
        } else {
            return Class.forName(name);
        }
    }

    protected Map<String, Class<?> > knownTypedefs = new HashMap<String, Class<?> >();

    void readTypedefs(org.w3c.dom.Element eElement) {
        for (org.w3c.dom.Element eTypedef : childElements(eElement, "typedef")) {
            String name = eTypedef.getAttribute("name");
            String type = eTypedef.getAttribute("type");
            try {
                Class<?> javaType = null;
                // Legacy. Is only used to read in typedefs.
                if (type.equals("BOOLEAN")) {
                    javaType = Boolean.class;
                } else {
                    javaType = getClass(type);
                }
                knownTypedefs.put(name, javaType);
            } catch (ClassNotFoundException e) {
                System.err.println("Unknown type " + type + ". Ignoring typedef for " + name);
            }
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
        for (org.w3c.dom.Element eAttribute : childElements(eElement, "attribute")) {
            String name  = eAttribute.getAttribute("type");
            Object value = eAttribute.getAttribute("value");
            Class<?> javaType = knownTypedefs.get(name);
            if (javaType != null) {
                value = toAttributeObject(eAttribute, javaType, (String) value);
            }
            attributes.put(name, value);
        }
        for (org.w3c.dom.Element eAttribute : childElements(eElement, "opendseattr")) {
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
        org.w3c.dom.Attr parameter = eAttribute.getAttributeNode("parameter");
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
        if (parameter != null) {
            if (parameter.getValue().equals("RANGE")) {
                return getRange(value.getValue());
            } else if (parameter.getValue().equals("DISCRETERANGE")) {
                return getRangeInt(value.getValue());
            } else if (parameter.getValue().equals("SELECT")) {
                return getSelectRefList(eAttribute, clazz, value.getValue());
            } else if (parameter.getValue().equals("UID")) {
                return getUniqueID(value.getValue());
            } else {
                throw new IllegalArgumentException("Unknown parameter type: " + parameter.getValue());
            }
        } else {
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
        for (org.w3c.dom.Element eNestedAttribute : childElements(eAttribute, "opendseattr")) {
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
        for (org.w3c.dom.Element eNestedAttribute : childElements(eAttribute, "opendseattr")) {
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

        if (Element.class.isAssignableFrom(clazz) && knownElements.containsKey(value)) {
            object = knownElements.get(value);
        } else {
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
//          if (object instanceof Element) {
//              knownElements.put(value, (Element) object);
//          }
        }
        if (object == null) {
            throw new IllegalArgumentException("type value mismatch for attribute " + eAttribute);
        }
        return object;
    }

    /**
     * Parse the {@link ParameterRange}.
     *
     * @param value
     *            the string to parse
     * @return the corresponding parameter
     */
    protected ParameterRange getRange(String value) {
        Scanner scanner = new Scanner(value);
        scanner.useDelimiter("[\\s+,()]+");

        double v = new Double(scanner.next());
        double lb = new Double(scanner.next());
        double ub = new Double(scanner.next());
        double gr = new Double(scanner.next());

        scanner.close();

        return Parameters.range(v, lb, ub, gr);
    }

    /**
     * Parse the {@link ParameterRangeDiscrete}.
     *
     * @param value
     *            the string to parse
     * @return the corresponding parameter
     */
    protected ParameterRangeDiscrete getRangeInt(String value) {
        Scanner scanner = new Scanner(value);
        scanner.useDelimiter("[\\s+,()]+");

        int v = new Integer(scanner.next());
        int lb = new Integer(scanner.next());
        int ub = new Integer(scanner.next());

        scanner.close();

        return Parameters.range(v, lb, ub);
    }

    /**
     * Parse the {@link ParameterSelect}.
     *
     * @param value
     *            the string to parse
     * @return the corresponding parameter
     */
    protected ParameterSelect getSelectRefList(org.w3c.dom.Element eAttribute, Class<?> clazz, String value) {
        value = value.replace("[", "(").replace("]", ")");

        Scanner scanner = new Scanner(value);
        scanner.useDelimiter("[()]+");

        Object def;
        List<Object> select = new ArrayList<Object>();
        try {
            def = toInstance(scanner.next(), clazz);
            for (String part : scanner.next().split(",")) {
                select.add(toInstance(part, clazz));
            }
        } catch (IllegalArgumentException | SecurityException | InstantiationException | IllegalAccessException
                | InvocationTargetException | NoSuchMethodException e) {
            scanner.close();
            throw new IllegalArgumentException("type value mismatch for attribute " + eAttribute);
        }

        String reference = null;
        if (scanner.hasNext()) {
            String next = scanner.next().trim();
            if (!next.equals("")) {
                reference = next;
            }
        }

        scanner.close();

        return Parameters.selectRefList(reference, def, select);
    }

    /**
     * Parse the {@link ParameterUniqueID}.
     *
     * @param value
     *            the string to parse
     * @return the corresponding parameter
     */
    protected ParameterUniqueID getUniqueID(String value) {
        Scanner scanner = new Scanner(value);
        scanner.findInLine("(\\w+) \\[UID:(\\w+)\\]");

        MatchResult result = scanner.match();
        int def = new Integer(result.group(1));
        String identifier = result.group(2);

        scanner.close();

        return Parameters.uniqueID(def, identifier);
    }

    /**
     * Gets an iterable list of child elements named {@param childName} of the
     * parent element {@param parentElement}.
     *
     * @param parentElement
     *            the parent element
     * @param childName
     *            the tag name of the desired child elements
     * @return the iterable element objects
     */
    protected static Iterable<org.w3c.dom.Element> childElements(final org.w3c.dom.Element parentElement, final String childName) {
        return new Iterable<org.w3c.dom.Element>() {

            @Override
            public Iterator<org.w3c.dom.Element> iterator() {
                return new Iterator<org.w3c.dom.Element>() {
                    private int c = -1;
                    private final org.w3c.dom.NodeList nodes = parentElement.getChildNodes();

                    {
                        skip();
                    }

                    private int skip() {
                        int old = c++;
                        while (hasNext() && !nodes.item(c).getNodeName().equals(childName))
                            ++c;
                        return old;
                    }

                    @Override
                    public boolean hasNext() {
                        return nodes.getLength() > c;
                    }

                    @Override
                    public org.w3c.dom.Element next() {
                        return (org.w3c.dom.Element) nodes.item(skip());
                    }

                    @Override
                    public void remove() {
                        throw new RuntimeException("invalid operation: remove");
                    }
                };
            }
        };
    }

    /**
     * Gets the single child element named {@param childName} of the parent
     * element {@param parentElement}. If there are more than one or no child
     * elements with the requested name, an exception is thrown.
     *
     * @param parentElement
     *            the parent element
     * @param childName
     *            the tag name of the desired child elements
     * @return the desired child element
     * @throws SNGFormatErrorException
     */
    protected static org.w3c.dom.Element childElement(final org.w3c.dom.Element parentElement, final String childName)
            throws SNGFormatErrorException {
        return childElement(parentElement, childName, false);
    }

    /**
     * Gets the single child element named {@param childName} of the parent
     * element {@param parentElement}. If there are more than one child element
     * with the requested name, an exception is thrown. If the element is
     * optional {@param optional} and not present, then null is returned.
     * Otherwise, if not optional and missing an exception is thrown.
     *
     * @param parentElement
     *            the parent element
     * @param childName
     *            the tag name of the desired child elements
     * @param optional
     *            If true, the element is allowed to be missing
     * @return the desired child element or null if optional and the element is
     *         missing
     * @throws SNGFormatErrorException
     */
    protected static org.w3c.dom.Element childElement(final org.w3c.dom.Element parentElement, final String childName,
            boolean optional) throws SNGFormatErrorException {
        Iterator<org.w3c.dom.Element> iter = childElements(parentElement, childName).iterator();
        if (!iter.hasNext()) {
            if (!optional)
                throw new SNGFormatErrorException(
                        "Parent element " + parentElement + " is missing a " + childName + " child element!");
            else
                return null;
        }
        org.w3c.dom.Element retval = iter.next();
        if (iter.hasNext())
            throw new SNGFormatErrorException(
                    "Parent element " + parentElement + " must only have one " + childName + " child element!");
        return retval;
    }
}
