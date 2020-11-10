/*******************************************************************************
 * Copyright (c) 2015 OpenDSE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *******************************************************************************/
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
import java.util.function.BiFunction;

import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.graph.util.Pair;
import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Attributes;
import net.sf.opendse.model.Communication;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Edge;
import net.sf.opendse.model.Element;
import net.sf.opendse.model.Function;
import net.sf.opendse.model.IAttributes;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Mappings;
import net.sf.opendse.model.Node;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Routings;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.model.parameter.ParameterRange;
import net.sf.opendse.model.parameter.ParameterRangeDiscrete;
import net.sf.opendse.model.parameter.ParameterSelect;
import net.sf.opendse.model.parameter.ParameterUniqueID;
import net.sf.opendse.model.parameter.Parameters;
import net.sf.opendse.optimization.SpecificationWrapper;

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
public class SNGReader implements SpecificationWrapper {

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
            org.w3c.dom.Element eSpec = doc.getDocumentElement();
            specification = toSpecification(eSpec);
            // nu.xom.Builder parser2 = new nu.xom.Builder();
            // nu.xom.Document doc2 = parser2.build(in);

            // nu.xom.Element eSpec = doc2.getRootElement();
        } catch (Exception ex) {
//          ex.printStackTrace(System.err);
            throw new SNGFormatErrorException(ex.getMessage());
        }
    }

    @Override
    public Specification getSpecification() {
        return specification;
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

    protected final Specification specification;

    /**
     * Convert a specification XML element to a specification
     * 
     * @param eSpec the specification XML element
     * @return the specification
     * @throws SNGFormatErrorException 
     */
    protected Specification toSpecification(org.w3c.dom.Element eSpec) throws SNGFormatErrorException {
//      System.err.println(eSpec.getNamespaceURI());
        org.w3c.dom.Element eArchitectureGraph = childElement(eSpec, "architecturegraph");
        org.w3c.dom.Element eProblemGraph;
        try {
            eProblemGraph = childElement(eSpec, "problemgraph");
        }
        catch(SNGFormatErrorException ex) {
            org.w3c.dom.Element eProcess = childElement(eSpec, "process");
            eProblemGraph = childElement(eProcess, "problemgraph");
            String nameAtt = eProcess.getAttribute("name");
            String idAtt = eProcess.getAttribute("id");
            eProblemGraph.setAttribute("name", nameAtt);
            eProblemGraph.setAttribute("id", idAtt);
        }
        org.w3c.dom.Element eMappings = childElement(eSpec, "mappings");
        org.w3c.dom.Element eRoutings = childElement(eSpec, "routings", true);
        
        readTypedefs(eSpec);

        Architecture<Resource, Link> architecture = toArchitecture(eArchitectureGraph);
        ProblemGraph problemGraph = toProblemGraph(eProblemGraph);
        Mappings<Task, Resource> mappings = toMappings(eMappings);
        
        Application<Task, Dependency> application = toApplication(problemGraph);

        Specification specification;

        if (eRoutings != null && eRoutings.getChildNodes().getLength() > 0) {
            assert false: "FIXME: Implement this!";
//          Routings<Task, Resource, Link> routings = toRoutings(eRoutings, architecture, application);
//          specification = new Specification(application, architecture, mappings, routings);
            specification = null;
        } else {
            specification = new Specification(application, architecture, mappings);
        }
        
        addAttributes(eSpec, specification);
        return specification;
    }

    protected static class ArchitectureGraphReader {
        
        protected final SNGReader sngReader;
        protected final Architecture<Resource, Link> architecture;
        protected final org.w3c.dom.Element eArchitectureGraph;

        protected enum PortDirection {
            In, Out
        };

        /// Map from the id of a port to the resource containing this port and
        /// the direction of the port.
        static class PortInfo {
            final public PortDirection direction;
            final public Resource resource;

            public PortInfo(PortDirection direction, Resource resource) {
                this.direction = direction;
                this.resource = resource;
            }
        }

        protected final HashMap<Long, PortInfo> portIdToPortInfo = new HashMap<Long, PortInfo>();

        public ArchitectureGraphReader(SNGReader sngReader, Architecture<Resource, Link> architecture,
                org.w3c.dom.Element eArchitectureGraph) throws SNGFormatErrorException {
            this.sngReader = sngReader;
            this.architecture = architecture;
            this.eArchitectureGraph = eArchitectureGraph;

            parseResources();
            parseEdges();
        }

        protected void parseResources() throws SNGFormatErrorException {
            for (org.w3c.dom.Element eResource : childElements(eArchitectureGraph, "resource")) {
                Resource resource = sngReader.createElement(eResource, Resource.class);
                architecture.addVertex(resource);
                for (org.w3c.dom.Element ePort : childElements(eResource, "port")) {
                    String directionStr = ePort.getAttribute("type");
                    assert directionStr.equals("in")
                            || directionStr.equals("out") : "Oops, illegal port direction for tag " + ePort;
                    PortDirection directionEnum = directionStr.equals("in") ? PortDirection.In : PortDirection.Out;
                    portIdToPortInfo.put(parseId(ePort), new PortInfo(directionEnum, resource));
                }
            }
        }

        protected void parseEdges() throws SNGFormatErrorException {
            final HashMap<Pair<Resource>, Link> links = new HashMap<Pair<Resource>, Link>();

            for (org.w3c.dom.Element eEdge : childElements(eArchitectureGraph, "edge")) {
                long idSource = parseId(eEdge, "source");
                long idTarget = parseId(eEdge, "target");
                PortInfo portInfoSource = portIdToPortInfo.get(idSource);
                PortInfo portInfoTarget = portIdToPortInfo.get(idTarget);
                if (portInfoSource == null) {
                    throw new SNGFormatErrorException("Source " + idSource + " of link " + eEdge + " not found!");
                }
                if (portInfoSource.direction != PortDirection.Out) {
                    throw new SNGFormatErrorException("Illegal port direction " + portInfoSource.direction
                            + " for source of link " + eEdge + "!");
                }
                if (portInfoTarget == null) {
                    throw new SNGFormatErrorException("Target " + idTarget + " of link " + eEdge + " not found!");
                }
                if (portInfoTarget.direction != PortDirection.In) {
                    throw new SNGFormatErrorException("Illegal port direction " + portInfoTarget.direction
                            + " for target of link " + eEdge + "!");
                }
                Link reverseLink = links.get(new Pair<Resource>(portInfoTarget.resource, portInfoSource.resource));
                // if we have edges r1 -> r2 and r2 -> r1, we will add an
                // undirected
                // edge between r1 and r2 to the architecture graph.
                if (reverseLink != null) {
                    architecture.removeEdge(reverseLink);
                    Link link = new Link(eEdge.getAttribute("id"));
                    architecture.addEdge(link, portInfoSource.resource, portInfoTarget.resource, EdgeType.UNDIRECTED);
                    links.put(new Pair<Resource>(portInfoSource.resource, portInfoTarget.resource), link);
                } else {
                    Link link = new Link(eEdge.getAttribute("id"));
                    architecture.addEdge(link, portInfoSource.resource, portInfoTarget.resource, EdgeType.DIRECTED);
                    links.put(new Pair<Resource>(portInfoSource.resource, portInfoTarget.resource), link);
                }
            }
        }

//      protected void parseLink(nu.xom.Element eLink, Architecture<Resource, Link> architecture)
//              throws ClassNotFoundException, InstantiationException, IllegalAccessException, InvocationTargetException,
//              NoSuchMethodException {
//          Link link = toEdge(eLink, null);
//
//          String type = eLink.getAttributeValue("orientation");
//          EdgeType edgeType = EdgeType.UNDIRECTED;
//          if (type != null) {
//              edgeType = EdgeType.valueOf(type);
//          }
//
//          String srcName = eLink.getAttributeValue("source");
//          Resource source = architecture.getVertex(srcName);
//          if (source == null) {
//              throw new IllegalArgumentException("Source of link " + link + " not found: " + srcName);
//          }
//
//          String dstName = eLink.getAttributeValue("destination");
//          Resource destination = architecture.getVertex(dstName);
//          if (destination == null) {
//              throw new IllegalArgumentException("Destination of link " + link + " not found: " + dstName);
//          }
//
//          architecture.addEdge(link, source, destination, edgeType);
//      } 
    };
    
    protected Architecture<Resource, Link> toArchitecture(org.w3c.dom.Element eArchitectureGraph) throws SNGFormatErrorException {
        Architecture<Resource, Link> architecture = new Architecture<Resource, Link>();
        new ArchitectureGraphReader (this, architecture, eArchitectureGraph);
        return architecture;
    }
    
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
    
    protected ProblemGraph toProblemGraph(org.w3c.dom.Element eProblemGraph) throws SNGFormatErrorException {
        ProblemGraph problemGraph = new ProblemGraph();   
        // Empty set of ports for the top problem graph.
        Map<Long, ProblemGraph.ConnectionInfo> pgPorts =  new HashMap<Long, ProblemGraph.ConnectionInfo>();        
        new ProblemGraphReader(problemGraph, pgPorts, eProblemGraph);
        return problemGraph;
    }
        
    protected Application<Task, Dependency> toApplication(ProblemGraph problemGraph) throws SNGFormatErrorException {
        Application<Task, Dependency> application = new Application<Task, Dependency>();       
        
        for (Task task: problemGraph.actors) {
            application.addVertex(task);            
        }

        // Maps a channel to a list of consumer actors.
        Map<Task, List<Task>> channelConnections = new HashMap<Task, List<Task>>();
        
        for (ProblemGraph.ConnectionInfo ci : problemGraph.connectionInfos) {
            if (ci.actorPortDirection == ProblemGraph.PortDirection.In)
                for (Task inputChannel : ci.channels) {                
                    channelConnections.compute(inputChannel, new BiFunction<Task, List<Task>, List<Task>>() {
                        public List<Task> apply(Task inputChannel, List<Task> consumerActors) {
                            if (consumerActors == null) {
                                consumerActors = new ArrayList<Task>();
                            }
                            consumerActors.add(ci.actor);
                            return consumerActors;
                        }
                    });
                }
        }
        for (ProblemGraph.ConnectionInfo ci : problemGraph.connectionInfos) {
            if (ci.actorPortDirection == ProblemGraph.PortDirection.Out) {
                Communication message = createElement(ci.eActorPort, Communication.class);
                {
                    Dependency dependency = new Dependency(createUniqeName());
                    application.addEdge(dependency, ci.actor, message, EdgeType.DIRECTED);
                }
                for (Task outputChannel : ci.channels) {
                    for (Task consumerActor : channelConnections.get(outputChannel)) {
                        Dependency dependency = new Dependency(createUniqeName());
                        application.addEdge(dependency, message, consumerActor, EdgeType.DIRECTED);                        
                    }
                }
            }
        }        
//        nu.xom.Elements eTasks = eApplication.getChildElements("task", Common.NS);
//        for (nu.xom.Element eTask : iterable(eTasks)) {
//            Task task = toNode(eTask, null);
//            application.addVertex(task);
//        }
//        nu.xom.Elements eCommunications = eApplication.getChildElements("communication", Common.NS);
//        for (nu.xom.Element eCommunication : iterable(eCommunications)) {
//            Communication communication = toNode(eCommunication, null);
//            application.addVertex(communication);
//        }
//
//        nu.xom.Elements eDependencies = eApplication.getChildElements("dependency", Common.NS);
//        for (nu.xom.Element eDependency : iterable(eDependencies)) {
//            parseDependency(eDependency, application);
//        }
//
//        nu.xom.Element eFunctions = eApplication.getFirstChildElement("functions", Common.NS);
//        if (eFunctions != null) {
//            nu.xom.Elements eFuncs = eFunctions.getChildElements("function", Common.NS);
//            for (nu.xom.Element eFunc : iterable(eFuncs)) {
//                Task task = application.getVertex(eFunc.getAttributeValue("anchor"));
//                Function<Task, Dependency> function = application.getFunction(task);
//                Attributes attributes = toAttributes(eFunc.getFirstChildElement("attributes", Common.NS));
//                setAttributes(function, attributes);
//            }
//        }
        return application;
    }
    
    

//    protected void parseDependency(nu.xom.Element eDependency, Application<Task, Dependency> application)
//            throws ClassNotFoundException, InstantiationException, IllegalAccessException, InvocationTargetException,
//            NoSuchMethodException {
//        Dependency dependency = toEdge(eDependency, null);
//
//        String srcName = eDependency.getAttributeValue("source");
//        Task source = application.getVertex(srcName);
//        if (source == null) {
//            throw new IllegalArgumentException("Source of dependency " + srcName + " not found: " + srcName);
//        }
//
//        String dstName = eDependency.getAttributeValue("destination");
//        Task destination = application.getVertex(dstName);
//        if (destination == null) {
//            throw new IllegalArgumentException("Destination of dependency " + dependency + " not found: " + dstName);
//        }
//
//        application.addEdge(dependency, source, destination, EdgeType.DIRECTED);
//    }

    protected Mappings<Task, Resource> toMappings(org.w3c.dom.Element eMappings) throws SNGFormatErrorException {
        Mappings<Task, Resource> mappings = new Mappings<Task, Resource>();

        for (org.w3c.dom.Element eMapping : childElements(eMappings, "mapping")) {
            Task     task     = getElement(eMapping,  "source",  Task.class);
            Resource resource = getElement(eMapping,  "target",  Resource.class);
            Mapping<Task, Resource> mapping = new Mapping<Task, Resource>(createUniqeName(eMapping), task, resource);
            addAttributes(eMapping, mapping);
            mappings.add(mapping);
        }
        return mappings;
    }
     
//    protected Routings<Task, Resource, Link> toRoutings(nu.xom.Element eRoutings,
//            Architecture<Resource, Link> architecture, Application<Task, Dependency> application)
//            throws IllegalArgumentException, SecurityException, ClassNotFoundException, InstantiationException,
//            IllegalAccessException, InvocationTargetException, NoSuchMethodException {
//        Routings<Task, Resource, Link> routings = new Routings<Task, Resource, Link>();
//
//        nu.xom.Elements eRoutingList = eRoutings.getChildElements("routing", Common.NS);
//        for (nu.xom.Element eRouting : iterable(eRoutingList)) {
//            String sourceId = eRouting.getAttributeValue("source");
//            Task source = application.getVertex(sourceId);
//
//            Architecture<Resource, Link> routing = toRouting(eRouting, architecture, application);
//            routings.set(source, routing);
//        }
//
//        return routings;
//    }
//
//    protected Architecture<Resource, Link> toRouting(nu.xom.Element eRouting, Architecture<Resource, Link> architecture,
//            Application<Task, Dependency> application)
//            throws IllegalArgumentException, SecurityException, ClassNotFoundException, InstantiationException,
//            IllegalAccessException, InvocationTargetException, NoSuchMethodException {
//        Map<String, Resource> map = new HashMap<String, Resource>();
//        Architecture<Resource, Link> routing = new Architecture<Resource, Link>();
//
//        nu.xom.Elements eResources = eRouting.getChildElements("resource", Common.NS);
//        for (nu.xom.Element eResource : iterable(eResources)) {
//            Resource parent = architecture.getVertex(eResource.getAttributeValue("id"));
//            Resource resource = toNode(eResource, parent);
//            routing.addVertex(resource);
//            map.put(resource.getId(), resource);
//        }
//
//        nu.xom.Elements eLinks = eRouting.getChildElements("link", Common.NS);
//        for (nu.xom.Element eLink : iterable(eLinks)) {
//            Link parent = architecture.getEdge(eLink.getAttributeValue("id"));
//            Link link = toEdge(eLink, parent);
//
//            String type = eLink.getAttributeValue("orientation");
//            EdgeType edgeType = EdgeType.UNDIRECTED;
//            if (type != null) {
//                edgeType = EdgeType.valueOf(type);
//            }
//
//            Resource source = map.get(eLink.getAttributeValue("source"));
//            Resource destination = map.get(eLink.getAttributeValue("destination"));
//
//            routing.addEdge(link, source, destination, edgeType);
//        }
//
//        return routing;
//    }
//    
//    @SuppressWarnings("unchecked")
//    protected <N extends Node> N toNode(nu.xom.Element eNode, N parent)
//            throws IllegalArgumentException, SecurityException, InstantiationException, IllegalAccessException,
//            InvocationTargetException, NoSuchMethodException, ClassNotFoundException {
//        Class<N> type = getClass(eNode);
//
//        N node = null;
//
//        if (parent == null) {
//            String id = eNode.getAttributeValue("id");
//            if (knownElements.containsKey(id)) {
//                node = (N) knownElements.get(id);
//            } else {
//                node = type.getConstructor(String.class).newInstance(id);
//                knownElements.put(node.getId(), node);
//            }
//        } else {
//            node = type.getConstructor(Element.class).newInstance(parent);
//        }
//
//        nu.xom.Elements eAttributes = eNode.getChildElements("attributes", Common.NS);
//        if (eAttributes.size() > 0) {
//            Attributes attributes = toAttributes(eAttributes.get(0));
//            setAttributes(node, attributes);
//        }
//
//        return node;
//
//    }
//
//    protected <E extends Edge> E toEdge(nu.xom.Element eEdge, E parent)
//            throws ClassNotFoundException, IllegalArgumentException, SecurityException, InstantiationException,
//            IllegalAccessException, InvocationTargetException, NoSuchMethodException {
//        Class<E> type = getClass(eEdge);
//
//        E edge = null;
//
//        if (parent == null) {
//            String id = eEdge.getAttributeValue("id");
//            edge = type.getConstructor(String.class).newInstance(id);
//        } else {
//            edge = type.getConstructor(Element.class).newInstance(parent);
//        }
//
//        nu.xom.Elements eAttributes = eEdge.getChildElements("attributes", Common.NS);
//        if (eAttributes.size() > 0) {
//            Attributes attributes = toAttributes(eAttributes.get(0));
//            setAttributes(edge, attributes);
//        }
//
//        return edge;
//    }    

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
