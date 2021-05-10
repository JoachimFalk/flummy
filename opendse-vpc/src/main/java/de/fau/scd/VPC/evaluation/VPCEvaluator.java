// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
 *   2020 FAU -- Martin Letras <martin.letras@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
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

package de.fau.scd.VPC.evaluation;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.opt4j.core.Objective;
import org.opt4j.core.Objectives;
import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.VPC.config.properties.ObjectiveInfo;
import de.fau.scd.VPC.helper.TempDirectoryHandler;
import de.fau.scd.VPC.io.VPCConfigReader;
import de.fau.scd.VPC.io.Common.FormatErrorException;
import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.ICommunication;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Mappings;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.ImplementationEvaluator;
import net.sf.opendse.model.Routings;

//import javax.xml.parsers.DocumentBuilder;
//import javax.xml.parsers.DocumentBuilderFactory;
//import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;


public class VPCEvaluator implements ImplementationEvaluator {

    public interface VPCObjectives {
        public List<ObjectiveInfo> getObjectives();
    }

    public interface SimInfo {
        public String    getExecutable();
        public String [] getArguments();
        public String [] getEnvironment();
        public String    getVpcConfigTemplate();
    }

    protected final String          simulatorExecutable;
    protected final String[]        simulatorArguments;
    protected final String[]        simulatorEnvironment;
    protected final VPCConfigReader vpcConfigTemplate;
    protected final boolean         vpcIgnoreRouting;
    protected final VPCObjectives   vpcObjectives;

    @Override
    public int getPriority() {
        return 10;
    }

    // Constructor of the VPC evaluator
    @Inject
    public VPCEvaluator(
        SimInfo       simInfo
      , @Constant(namespace = VPCEvaluatorModule.class, value = "vpcIgnoreRouting")
        boolean       vpcIgnoreRouting
      , VPCObjectives vpcObjectives
      ) throws
        FileNotFoundException
      , FormatErrorException
    {
        this.simulatorExecutable    = simInfo.getExecutable();
        this.simulatorArguments     = simInfo.getArguments();
        this.simulatorEnvironment   = simInfo.getEnvironment();
        this.vpcConfigTemplate      = new VPCConfigReader(simInfo.getVpcConfigTemplate());
        this.vpcIgnoreRouting       = vpcIgnoreRouting;
        this.vpcObjectives          = vpcObjectives;
    }

    @Override
    public Specification evaluate(Specification implementation, Objectives objectives) {
        boolean error = false;

        // Extract the working directory
        TempDirectoryHandler tempDirectoryHandler = TempDirectoryHandler.getTempDirectoryHandler(implementation);

        org.w3c.dom.Document vpcDocument = (org.w3c.dom.Document) vpcConfigTemplate.getDocument().cloneNode(true);

        org.w3c.dom.Element eVPCConfig = vpcDocument.getDocumentElement();
        assert eVPCConfig.getNodeName() == "vpcconfiguration";
        try {
            org.w3c.dom.Element eResources = VPCConfigReader.childElement(eVPCConfig, "resources");
            processResources(implementation, eResources);
            org.w3c.dom.Element eLinks = VPCConfigReader.childElement(eVPCConfig, "links", true);
            if (eLinks != null)
                processLinks(implementation, eLinks);
            org.w3c.dom.Element eMappings = VPCConfigReader.childElement(eVPCConfig, "mappings");
            processMappings(implementation, eMappings);
            org.w3c.dom.Element eTopology = VPCConfigReader.childElement(eVPCConfig, "topology", vpcIgnoreRouting);
            if (vpcIgnoreRouting) {
                org.w3c.dom.Element eTopologyIgnore =
                    vpcDocument.createElement("topology");
                eTopologyIgnore.setAttribute("default", "ignore");
                if (eTopology != null) {
                    eVPCConfig.replaceChild(eTopologyIgnore, eTopology);
                } else {
                    eVPCConfig.appendChild(eTopologyIgnore);
                }
            } else {
                processTopology(implementation, eTopology);
            }
        } catch (FormatErrorException ex) {
            ex.printStackTrace();
            error = true;
        }

        File outputVPCConfig = new File(tempDirectoryHandler.getDirectory(), "vpc.config.xml");

        try {
            TransformerFactory transformerFactory = TransformerFactory.newInstance();
            Transformer transf = transformerFactory.newTransformer();

            transf.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
            transf.setOutputProperty(OutputKeys.INDENT, "yes");
            transf.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2");
            org.w3c.dom.DocumentType docType = vpcDocument.getImplementation()
                    .createDocumentType("doctype", null, "vpc.dtd");
//          transf.setOutputProperty(OutputKeys.DOCTYPE_PUBLIC, docType.getPublicId());
            transf.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, docType.getSystemId());
            DOMSource source = new DOMSource(vpcDocument);
    //      transf.transform(source, new StreamResult(System.out));
            transf.transform(source, new StreamResult(outputVPCConfig));
        } catch (TransformerException ex) {
            ex.printStackTrace();
            error = true;
        }

//      // Create the VPC configuration
//      Element root = new Element(VPCCONFIGURATION);
//      // Extract the architecture graph
//      Element resources = processResources(implementation);
//      // Write the architecture graph in the VPC configuration file!
//      root.addContent(resources);
//      // Write the mappings in the VPC file!
//      root.addContent(processMappings(implementation));
//      Element topology = new Element(TOPOLOGY);
//      topology.setAttribute("default","ignore");
//      root.addContent(topology);
//      Routings<Task, Resource, Link> oneToOneRoutings = extractRouting(implementation);

        //root.addContent(routingsToJdom(oneToOneRoutings, implementation, resources));

//      double simTime = Double.MAX_VALUE;

        {
            try {
                ArrayList<String> cmd = new ArrayList<String>();
                cmd.add(this.simulatorExecutable);
                cmd.add("--systemoc-vpc-config");
                cmd.add(outputVPCConfig.getCanonicalPath());
                for (String arg : simulatorArguments) {
                    cmd.add(arg);
                }
                Process simProcess = Runtime.getRuntime().exec(
                    cmd.toArray(new String[0]),
                    simulatorEnvironment,
                    tempDirectoryHandler.getDirectory());
                int status = simProcess.waitFor();
                if (status != 0) {
                    error = true;
                    System.err.print("Simulation");
                    for (String arg : cmd) {
                        System.err.print(" "+arg);
                    }
                    System.err.println(" terminated with status "+status+"!");
                    System.err.println("==== ENV =====");
                    for (String env : simulatorEnvironment) {
                        System.err.println(env);
                    }
                    System.err.println("==============");
                    BufferedReader stdout = new BufferedReader(
                        new InputStreamReader(simProcess.getInputStream()));
                    for (String line = stdout.readLine();
                         line != null;
                         line = stdout.readLine()) {
                        System.err.println(line);
                    }
                    BufferedReader stderr = new BufferedReader(
                        new InputStreamReader(simProcess.getErrorStream()));
                    for (String line = stderr.readLine();
                         line != null;
                         line = stderr.readLine()) {
                        System.err.println(line);
                    }
                    System.err.println("==============");
                }
            } catch (IOException e) {
                System.err.println("Got an exception during I/O with simulation:\n" + e);
                error = true;
            } catch (InterruptedException e) {
                System.err.println("Got an exception during simulation:\n" + e);
                error = true;
            }
        }
        for (ObjectiveInfo objInfo : vpcObjectives.getObjectives()) {
            Double objValue = null;

            try {
                File parseFile = new File(
                    tempDirectoryHandler.getDirectory().getPath()
                  , objInfo.getParseFile().getPath());
                Pattern regex = objInfo.getParseRegex();
                BufferedReader br = new BufferedReader(
                        new FileReader(parseFile));
                for (String line = br.readLine();
                     line != null;
                     line = br.readLine()) {
                    Matcher m = regex.matcher(line);
                    if (m.find()) {
                        try {
                            objValue = Double.parseDouble(m.group(1));
                        } catch (IndexOutOfBoundsException e) {
                            System.err.println("For objective " + objInfo.getObjName()
                                + ": Regex \"" + regex.pattern() + "\" is missing a capture group!");
                        } catch (NumberFormatException e) {
                            System.err.println("For objective " + objInfo.getObjName()
                                + ": Can't parse " + m.group(1) + " as a double!");
                        }
                        break;
                    }
                }
                br.close();
            } catch (FileNotFoundException e) {
                System.err.println("Got an exception during reading results:\n" + e);
            } catch (IOException e) {
                System.err.println("Got an exception during reading results:\n" + e);
            }
            if (error || objValue == null)
                objectives.add(objInfo.getObjective(), Objective.INFEASIBLE);
            else
                objectives.add(objInfo.getObjective(), objValue);
        }

        return implementation;
    }

    // Method that extracts the architecture graph and specifies the scheduler
    protected void processResources(
        Specification       implementation
      , org.w3c.dom.Element eResources
      )
    {
        Map<String, Resource> nameToResource = new HashMap<String, Resource>();

        for (Resource resource : implementation.getArchitecture().getVertices()) {
            String name = resource.getAttribute("name");
            if (name == null)
                name = resource.getId();
            Resource old = nameToResource.put(name, resource);
            assert old == null : "Duplicate resource name " + name;
        }

        {
            Iterator<org.w3c.dom.Element> eComponentIter =
                    VPCConfigReader.childElements(eResources, "component").iterator();
            while (eComponentIter.hasNext()) {
                org.w3c.dom.Element eComponent = eComponentIter.next();
                String name = eComponent.getAttribute("name");
                Resource resource = nameToResource.get(name);
                if (resource == null) {
                    // Resource is missing; Remove it from VPC configuration file.
                    eComponentIter.remove();
                    continue;
                }
                nameToResource.remove(name);
            }
        }
        assert nameToResource.isEmpty();

//      // Define list of resources
//      Element resources = new Element(RESOURCES);
//      // Iterate the resources in the architecture graph
//      for(Resource resource : implementation.getArchitecture().getVertices()) {
//          String name = resource.getAttribute(NAMESTR);
//
//          String scheduler = "RRNOPRE";
//          Element component = new Element(COMPONENT);
//
//          // set the name of the Resource
//          component.setAttribute(NAMESTR, name);
//          // set the scheduler
//          component.setAttribute(SCHEDULER, scheduler);
//          //  it is necessary to include attributes according to the scheduler
//          resources.addContent(component);
//
//          // Set the visualizator for the traces!
//
//          Element tracing = new Element(ATTRIBUTE);
//          tracing.setAttribute(TYPE, TRACING);
//          tracing.setAttribute(VALUE, this.traceType.toString());
//
//          component.addContent(tracing);
//
//          Element sched = new Element(ATTRIBUTE);
//          Element subElement = new Element(ATTRIBUTE);
//
//          switch(this.schedulerType) {
//              case TDMA:
//                  // code block
//                  break;
//              /**
//               * Use FLEXRAY scheduler
//               */
//              case FLEXRAY:
//                  // code block
//                  break;
//              /**
//               * Use TTCC scheduler
//               */
//              case TTCC:
//
//                  break;
//              /**
//               * Use Round Robin scheduler
//               */
//              case RR:
//                  sched.setAttribute(TYPE, SCHEDULER);
//
//                  subElement.setAttribute(TYPE, "timeslice");
//                  subElement.setAttribute(VALUE, Double.toString(this.timeSlice));
//
//                  sched.addContent(subElement);
//                  component.addContent(sched);
//                  break;
//              /**
//               * Use Round Robin no-preemption scheduler
//               */
//              case RRNOPRE:
//                  sched.setAttribute(TYPE, SCHEDULER);
//
//                  subElement.setAttribute(TYPE, "fireActorInLoop");
//                  if (this.fireActorInLoop)
//                      subElement.setAttribute(VALUE, "true");
//                  else
//                      subElement.setAttribute(VALUE, "false");
//
//                  sched.addContent(subElement);
//                  component.addContent(sched);
//                  break;
//              /**
//               * Use Static Priority scheduler
//               */
//              case SP:
//                  break;
//              /**
//               * Use Static Priority no-preemption scheduler
//               */
//              case SPNOPRE:
//                  break;
//              /**
//               * Use First come first served scheduler
//               */
//              case FCFS:
//                  break;
//              /**
//               * Use Rate monotonic scheduler
//               */
//              case RM:
//                  break;
//              /**
//               * Use AVB scheduler
//               */
//              case AVB:
//                  break;
//              /**
//               * Use MOST scheduler
//               */
//              case MOST:
//                  break;
//              /**
//               * Use STREAMSHAPER scheduler
//               */
//              case STREAMSHAPER:
//                  break;
//          }
//      }
//
//      return resources;
    }

    protected void processLinks(
        Specification       implementation
      , org.w3c.dom.Element eLinks
      )
    {
        Set<String> resources = new HashSet<>();
        for (Resource res : implementation.getArchitecture().getVertices()) {
            resources.add(res.getId());
        }
        Iterator<org.w3c.dom.Element> eLinkIter =
                VPCConfigReader.childElements(eLinks, "link").iterator();
        while (eLinkIter.hasNext()) {
            org.w3c.dom.Element eLink = eLinkIter.next();
            org.w3c.dom.Attr source = eLink.getAttributeNode("source");
            org.w3c.dom.Attr target = eLink.getAttributeNode("target");
            if (source != null && !resources.contains(source.getNodeValue()) ||
                target != null && !resources.contains(target.getNodeValue())) {
                // Source or target Resource is missing; Remove link from VPC configuration file.
                eLinkIter.remove();
                continue;
            }
        }
    }

    protected void processMappings(
        Specification       implementation
      , org.w3c.dom.Element eMappings
      )
    {
        class SourceTarget {
            public SourceTarget(String source, String target) {
                this.source = source;
                this.target = target;
            }

            public String getSource() {
                return source;
            }
            public String getTarget() {
                return target;
            }

            final String source;
            final String target;
        };

        Map<SourceTarget, Mapping<Task, Resource>> sourceTargetToMapping =
                new HashMap<SourceTarget, Mapping<Task, Resource>>();

        for (Mapping<Task, Resource> mapping : implementation.getMappings()) {
            String source = mapping.getSource().getAttribute("name");
            if (source == null)
                source = mapping.getSource().getId();
            String target = mapping.getTarget().getAttribute("name");
            if (target == null)
                target = mapping.getTarget().getId();

            SourceTarget sourceTarget = new SourceTarget(source, target);
            Mapping<Task, Resource> old = sourceTargetToMapping.put(sourceTarget, mapping);
            assert old == null : "Duplicate mapping from " + source + " to " + target;
        }

        {
            Iterator<org.w3c.dom.Element> eMappingIter =
                    VPCConfigReader.childElements(eMappings, "mapping").iterator();
            while (eMappingIter.hasNext()) {
                org.w3c.dom.Element eComponent = eMappingIter.next();
                String source = eComponent.getAttribute("source");
                String target = eComponent.getAttribute("target");
                SourceTarget sourceTarget = new SourceTarget(source, target);
                Mapping<Task, Resource> mapping = sourceTargetToMapping.get(sourceTarget);
                if (mapping == null) {
                    // Mapping is missing; Remove it from VPC configuration file.
                    eMappingIter.remove();
                    continue;
                }
                sourceTargetToMapping.remove(sourceTarget);
            }
        }
        //assert sourceTargetToMapping.isEmpty();
        for (Map.Entry<SourceTarget, Mapping<Task, Resource> > entry :
                sourceTargetToMapping.entrySet())
        {
            org.w3c.dom.Document vpcDocument = eMappings.getOwnerDocument();
            org.w3c.dom.Element eMapping = vpcDocument.createElement("mapping");
            {
                org.w3c.dom.Attr aSource = vpcDocument.createAttribute("source");
                aSource.setNodeValue(entry.getKey().getSource());
                eMapping.setAttributeNode(aSource);
                org.w3c.dom.Attr aTarget = vpcDocument.createAttribute("target");
                aTarget.setNodeValue(entry.getKey().getTarget());
                eMapping.setAttributeNode(aTarget);
                eMappings.appendChild(eMapping);
            }
            for (Map.Entry<String, Object> attr : entry.getValue().getAttributes().entrySet()) {
                if (!attr.getKey().startsWith("vpc-")) {
                    // Ignore non vpc-* attributes
                    continue;
                }
                if (attr.getKey().equals("vpc-actor-delay")) {
                    if (attr.getValue() instanceof Map) {
                        @SuppressWarnings("unchecked")
                        Map<String, Double> atts = (Map<String, Double>) attr.getValue();
                        for (Map.Entry<String, Double> timing : atts.entrySet()) {
                            org.w3c.dom.Element eTiming = vpcDocument.createElement("timing");
                            if (!timing.getKey().isEmpty()) {
                                org.w3c.dom.Attr aFName = vpcDocument.createAttribute("fname");
                                aFName.setNodeValue(timing.getKey());
                                eTiming.setAttributeNode(aFName);
                            }
                            org.w3c.dom.Attr aDelay = vpcDocument.createAttribute("delay");
                            aDelay.setNodeValue(Double.toString(timing.getValue() * 1E9)+" ns");
                            eTiming.setAttributeNode(aDelay);
                            eMapping.appendChild(eTiming);
                        }
                    }
                } else {
                    throw new RuntimeException("Mapping attribute "+attr.getKey()+" not supported by VPCEvaluator!");
                }
            }
        }

//      Application application = implementation.getApplication();
//      Element mappings = new Element("mappings");
//
//      Set<Cluster> clusters = implementation.getAttribute(DirectImplementationTransformer.TAG);
//      for(Cluster clu : clusters) {
//          //System.err.println("Clusters "+clu.getId());
//          // Get all the clustered actors in each cluster
//          Collection<Task> clusteredActors = clu.getClusteredActors();
//          if (clusteredActors != null) {
//              // Get the Resource in the Cluster
//              Resource resource = null;
//              for (Task clusteredActor: clusteredActors) {
//                  for(Mapping<Task,Resource> imapping : implementation.getMappings().get(clusteredActor)) {
//                      resource = imapping.getTarget();
//                  }
//              }
//              // write cluster information into the VPC file!
//              if (resource != null) {
//               //   System.err.println("CLUSTER "+clu.getId()+ " MAPPED TO RESOURCE : "+resource.<String>getAttribute("name"));
//               // create a mapping tag for each cluster
//                  Element mapping = new Element("mapping");
//                  mappings.addContent(mapping);
//                  mapping.setAttribute("source", "pgTop."+clu.getId());
//                  mapping.setAttribute("target", resource.<String>getAttribute("name"));
//
//                  Element vpc_timing = new Element("timing");
//                  mapping.addContent(vpc_timing);
//                  vpc_timing.setAttribute("delay","0 ns");
//              }
//          }
//
//      }
//      for (Task task : (Collection<Task>)application.getVertices()) {
//          //if (task instanceof IFunction && !(systemoc.isFifo(task))) {
//          if (!(task instanceof ICommunication)) {
//              // found ACTOR: create mapping
//              Collection<Mapping<Task,Resource>> binding = implementation.getMappings().get(task);
//              // System.err.println(task.getName());
//              for (Mapping<Task, Resource> imapping : binding) {
//                  // Mapping m = (Mapping) imapping;
//                  Resource resource = imapping.getTarget();
//
//                  // create mapping tag for CMX file
//                  Element mapping = new Element("mapping");
//                  mappings.addContent(mapping);
//                  if (task.<String>getAttribute("name") != null) {
//                      mapping.setAttribute("source", task.<String>getAttribute("name"));
//                  } else {
//                      mapping.setAttribute("source", task.<String>getAttribute("name"));
//                  }
//                  mapping.setAttribute("target", resource.<String>getAttribute("name"));
//
//                  //Read all the attributes inside each mapping
//                  //Such attributes are the timing annotation
//                  HashMap<String,Object> atts = imapping.getAttribute("vpc-actor-delay");
//                  double timing = 0.0;
//
//                  for (HashMap.Entry<String,Object> entry : atts.entrySet()) {
//                      timing += (double)entry.getValue();
//                  }
//
//                  Element vpc_timing = new Element("timing");
//                  mapping.addContent(vpc_timing);
//                  // The specifaction contains times in seconds
//                  String vpcDelay = (timing/ 1000000000) +" ns";
//                  vpc_timing.setAttribute("delay", vpcDelay);
//              }
//          }
//      }
//      return mappings;
    }

    static protected class Routing {
        public final Task message;
        public final Architecture<Resource, Link> routing;

        public Routing(Task message, Architecture<Resource, Link> routing) {
            this.message = message;
            this.routing = routing;
        }
    }

    protected void processTopology(
        Specification       implementation
      , org.w3c.dom.Element eTopology
      )
    {
        Map<String, Routing> routes = new HashMap<>();
        Routings<Task, Resource, Link> routings = implementation.getRoutings();
        for (Task message : routings.getTasks()) {
            assert message instanceof ICommunication;
            routes.put(message.getId(), new Routing(message, routings.get(message)));
        }
        Iterator<org.w3c.dom.Element> eRouteIter =
            VPCConfigReader.childElements(eTopology, "route").iterator();
        while (eRouteIter.hasNext()) {
            org.w3c.dom.Element eRoute = eRouteIter.next();
            String name = eRoute.getAttribute("name");
            Routing routing = routes.get(name);
            if (routing == null) {
                // Message is not routed; Remove route from VPC configuration file.
                eRouteIter.remove();
                continue;
            }
            {
                org.w3c.dom.Node child;
                while ((child = eRoute.getFirstChild()) != null)
                    eRoute.removeChild(child);
            }
            processRoute(implementation, routing, eRoute);
            routes.remove(name);
        }

        org.w3c.dom.Document vpcDocument = eTopology.getOwnerDocument();

        for (Routing routing : routes.values()) {
            org.w3c.dom.Element eRoute = vpcDocument.createElement("route");
            eRoute.setAttribute("name", routing.message.getId());
            eTopology.appendChild(eRoute);
            processRoute(implementation, routing, eRoute);
        }
    }

    protected void processRoute(
            Specification       implementation
          , Routing             routing
          , org.w3c.dom.Element eRoute
          )
    {
        Application<Task, Dependency> application = implementation.getApplication();
        Mappings<Task, Resource> mappings = implementation.getMappings();

        eRoute.setAttribute("type", "StaticRoute");

  /*  <route name="sqrroot.a1.o1" type="StaticRoute" tracing="false">
        <hop component="CPU">
          <hop component="Bus">
            <hop component="Mem">
              <desthop/>
            </hop>
          </hop>
        </hop>
      </route>
   */
        Collection<Task> predecessor = application.getPredecessors(routing.message);
        assert predecessor.size() == 1;
        Set<Mapping<Task, Resource> > srcMapping = mappings.get(predecessor.iterator().next());
        assert srcMapping.size() == 1;
        Resource srcHop =  srcMapping.iterator().next().getTarget();

    }

//  private Element routingsToJdom( Routings<Task,Resource,Link> oneToOneRoutings, Specification implementation, Element resources) {
//      Element topology = new Element(TOPOLOGY);
//      topology.setAttribute("tracing", "true");
//
//      Iterator<Task> iter = oneToOneRoutings.getTasks().iterator();
//
//      // loop over messages
//      while (iter.hasNext()) {
//          Task message = iter.next();
//          Iterator<Architecture<Resource,Link>> siter = oneToOneRoutings.getRoutings().iterator();
//
//          // loop over every routing for a message (>1 if multicast)
//          // Collection<IResource> hopsUsed = new LinkedList<IResource>();
//          while (siter.hasNext()) {
//              Architecture<Resource,Link> routing = siter.next();
//    //        System.out.println("routing: " + routing.toString());
//              routingToJdom(implementation, resources, topology, message, routing);
//          }
//      }
//      return topology;
//  }
//
//  private void routingToJdom(Specification implementation, Element resources,Element topology, Task message, Architecture<Resource,Link> routing) {
//
//      Collection<Task> preds = implementation.getApplication().getPredecessors(message);
//      assert preds.size() == 1: "A message must only have one predecessor";
//      Task predecessor = preds.iterator().next();
//
//    //assert implementation.getRoutings().get(task) .getBindings().get(predecessor).size() == 1 : "Did not expect task to be bound to more than one resource";
//
//      // find the right succ task
//      for (Task successor : implementation.getApplication().getSuccessors(message)) {
//    //    System.out.println("source " + task.toString());
//          //assert implementation.getBindings().get(successor).size() == 1 : "Did not expect task to be bound to more than one resource";
//    //    IResource target = implementation.getBindings().get(task).iterator().next().getTarget();
//    //    System.out.println("target " + target.toString());
//
//          String msgName = message.<String> getAttribute("NAME");
//          // Depending on the way the model is read into DSE, a message can either be a FIFO
//          // or an edge connecting an actor to FIFO. Please refer to SysteMoCXMLInputOutput for
//          // details. If the name of the message ends with "_push" this should be the connection
//          // from the actor to the FIFO, otherwise the message ends with "_pull" representing
//          // the edge from the FIFO to the actor.
//
//          if (msgName.endsWith("_push")) {
//              Element pushRoute = new Element("route");
//              topology.addContent(pushRoute);
//              pushRoute.setAttribute("source", predecessor.<String> getAttribute("NAME"));
//              pushRoute.setAttribute("destination", successor.<String> getAttribute("NAME"));
//          } else if (msgName.endsWith("_pull")) {
//              Element pullRoute = new Element("route");
//              topology.addContent(pullRoute);
//              pullRoute.setAttribute("source", predecessor.<String> getAttribute("NAME"));
//              pullRoute.setAttribute("destination", successor.<String> getAttribute("NAME"));
//          } else {
//              // The message is a FIFO, create two route elements for VPC.
//              Element pushRoute = new Element("route");
//              topology.addContent(pushRoute);
//              pushRoute.setAttribute("source", predecessor.<String> getAttribute("NAME"));
//              pushRoute.setAttribute("destination", msgName);
//
//              Element pullRoute = new Element("route");
//              topology.addContent(pullRoute);
//              pullRoute.setAttribute("source", msgName);
//              pullRoute.setAttribute("destination", successor.<String> getAttribute("NAME"));
//          }
//      }
//  }

    /**
     * Extracts one to one routings from the implementation, i.e. hops on a one to one route have 0 or 1
     * predecessor/successor. E.g. Unicast routes stay as is and multicast messages will be split into their different
     * unicast equivalents
     *
     * @param implementation
     * @return Map with routings per message
     */
    //private Map<Communication, List<Routing<IResource>>> extractRouting(Specification implementation) {
  /*
    private Map<ICommunication, List<Routing<IResource>>> extractRouting(
            IImplementation<ITask, IResource, IMapping> implementation) {

        Collection<ITask> receivingFunctions = getReceivingFunctions(implementation);
          Map<ICommunication, List<Routing<IResource>>> oneToOneRoutings = new Hashtable<ICommunication, List<Routing<IResource>>>();
          // traverse routing from each receiving function
          for (ITask task : receivingFunctions) {
            // find messages task depends on
            Collection<ITask> messages = implementation.getApplication().getPredecessors(task);

            for (ITask message : messages) {
              // if message is already in 1:1 routings get List
              List<Routing<IResource>> routingList;

              if (oneToOneRoutings.containsKey(message)) {
                routingList = oneToOneRoutings.get(message);
              } else {
                routingList = new LinkedList<Routing<IResource>>();
                oneToOneRoutings.put((ICommunication) message, routingList);
              }
              // get last hop and build routing
              // TODO: tasks are only mapped to one resource?!
              Routing<IResource> oneToOneRouting = new Routing<IResource>();
              routingList.add(oneToOneRouting);
            }
          }
          return oneToOneRoutings;
        }

    */
    private Routings<Task, Resource, Link>  extractRouting(Specification implementation) {
        Collection<Task> receivingFunctions = getReceivingFunctions(implementation);
        //Routings<Task,Resource,Link> oneToOneRoutings= new Routings<Task,Resource,Link>();
        Routings<Task, Resource, Link> oneToOneRoutings = new Routings<Task, Resource, Link>();

        // traverse routing from each receiving function
        for (Task task : receivingFunctions) {
            // find messages task depends on
            Collection<Task> messages = implementation.getApplication().getPredecessors(task);
            for (Task message : messages) {
                // if message is already in 1:1 routings get List
                Architecture<Resource, Link> routing;

                if(oneToOneRoutings.getTasks().contains(message)) {
                    routing = oneToOneRoutings.get(message);
                }
                else {
                    routing = new Architecture<Resource, Link>();
                    oneToOneRoutings.set(task, routing);
                }
            }
        }
        return oneToOneRoutings;
    }

    private Collection<Task> getReceivingFunctions(Specification implementation) {
        Application<Task,Dependency> application = implementation.getApplication();

        Collection<Task> receivingFunctions = new ArrayList<Task>();

        for (Task task : application) {
            //if (task instanceof IFunction) {
            if (!(task instanceof ICommunication)) {
                if (application.getInEdges(task).size() > 0) {
                    receivingFunctions.add(task);
                }
            }
        }
        return receivingFunctions;
    }
}

