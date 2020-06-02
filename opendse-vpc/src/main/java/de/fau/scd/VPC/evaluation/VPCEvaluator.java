// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
 *   2020 FAU -- Martin Letras <martin.letras@fau.de>
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

package de.fau.scd.VPC.evaluation;

import static java.lang.annotation.RetentionPolicy.RUNTIME;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.annotation.Retention;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.StringTokenizer;

import javax.sound.midi.MetaMessage;

import org.apache.commons.collections15.functors.InstantiateFactory;
import org.apache.commons.collections15.map.LazyMap;
import org.jdom.Attribute;
import org.jdom.DocType;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import org.opt4j.core.DoubleValue;
import org.opt4j.core.Objective;
import org.opt4j.core.Objectives;
import org.opt4j.core.Value;
import org.opt4j.core.Objective.Sign;
import org.opt4j.core.problem.Evaluator;
import org.opt4j.core.start.Constant;

import com.google.common.collect.Iterables;
import com.google.inject.BindingAnnotation;
import com.google.inject.Inject;

import de.fau.scd.VPC.evaluation.VPCEvaluatorModule.SchedulerTypeEnum;
import de.fau.scd.VPC.evaluation.VPCEvaluatorModule.TraceTypeEnum;
import de.fau.scd.VPC.helper.TempDirectoryHandler;
import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Attributes;
import net.sf.opendse.model.Communication;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Function;
import net.sf.opendse.model.ICommunication;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.ImplementationEvaluator;
import net.sf.opendse.model.Routings;


public class VPCEvaluator implements ImplementationEvaluator {

    Objective SimTime = new Objective("SimTime[ns]", Objective.Sign.MIN);

    @Retention(RUNTIME)
    @BindingAnnotation
    public @interface ApplicationGraph {
    }

    @Retention(RUNTIME)
    @BindingAnnotation
    public @interface ExecutableOfSimulation {
    }

    @Retention(RUNTIME)
    @BindingAnnotation
    public @interface NumberOfIterations {
    }

    @Retention(RUNTIME)
    @BindingAnnotation
    public @interface SchedulerType {
    }

    @Retention(RUNTIME)
    @BindingAnnotation
    public @interface TimeSlice{
    }

    @Retention(RUNTIME)
    @BindingAnnotation
    public @interface FireActorInLoop{
    }

    @Retention(RUNTIME)
    @BindingAnnotation
    public @interface TraceType{
    }

    protected final String applicationGraph;

    protected final String executableOfSimulation;

    protected final int numberOfIterations;

    protected final SchedulerTypeEnum schedulerType;

    protected final double timeSlice;

    protected final boolean fireActorInLoop;

    protected final TraceTypeEnum traceType;

    Objective objective =  new Objective("#throughput VPC", Sign.MAX);

    // Labels for generating the VPC configuration file
    static protected final String configFileName = "config.xml";

    static protected final String dtdFileName = "vpc.dtd";

    static protected final String VPCCONFIGURATION = "vpcconfiguration";

    static protected final String RESOURCES = "resources";

    static protected final String SCHEDULER = "scheduler";

    static protected final String COMPONENT = "component";

    static protected final String MAPPINGS = "mappings";

    static protected final String MAPPING = "mapping";

    static protected final String DISTRIBUTION = "distribution";

    static protected final String TRACING = "tracing";

    static protected final String TYPE = "type";

    static protected final String VALUE = "value";

    static protected final String NAMESTR = "name";

    static protected final String ATTRIBUTE = "attribute";

    static protected final String TOPOLOGY = "topology";

    static protected final String PARAMETER = "parameter";


    // Constructor of the VPC evaluator
    @Inject
    public VPCEvaluator(@ExecutableOfSimulation String executableOfSimulation, @NumberOfIterations int numberOfIterations,
                        @SchedulerType SchedulerTypeEnum schedulerType, @TimeSlice double timeSlice,
                        @FireActorInLoop boolean fireActorInLoop, @TraceType TraceTypeEnum traceType,@ApplicationGraph String applicationGraph) {
        this.executableOfSimulation         = executableOfSimulation;
        this.numberOfIterations             = numberOfIterations;
        this.schedulerType                  = schedulerType;
        this.timeSlice                      = timeSlice;
        this.fireActorInLoop                = fireActorInLoop;
        this.traceType                      = traceType;
        this.applicationGraph               = applicationGraph;
    }

    @Override
    public Specification evaluate(Specification implementation, Objectives objectives) {
        // Extract the working directory
        TempDirectoryHandler tempDirectoryHandler   = TempDirectoryHandler.getTempDirectoryHandler(implementation);
        // Define the objective
        Value<?> objectiveValue                     = Objective.INFEASIBLE;
        // Create the VPC configuration
        Element root = new Element(VPCCONFIGURATION);
        // Extract the architecture graph
        Element resources = processResources(implementation);
        // Write the architecture graph in the VPC configuration file!
        root.addContent(resources);
        // Write the mappings in the VPC file!
        root.addContent(processMappings(implementation));

        Element topology = new Element(TOPOLOGY);
        topology.setAttribute("default","ignore");
        root.addContent(topology);
        Routings<Task, Resource, Link> oneToOneRoutings = extractRouting(implementation);

        //root.addContent(routingsToJdom(oneToOneRoutings, implementation, resources));


        DocType vpcXml = new DocType(VPCCONFIGURATION, dtdFileName);
        Document doc = new Document(root, vpcXml);

        double simTime = Double.MAX_VALUE;

        try {
            // generate temporary directory
            File clusterDir = tempDirectoryHandler.getDirectory();
            // generate VPC config
            File confFile = new File(clusterDir, configFileName);
            BufferedWriter bw = new BufferedWriter(new FileWriter(confFile));

            XMLOutputter serializer = new XMLOutputter(Format.getPrettyFormat());
            serializer.output(doc, bw);
            bw.close();

            String cmd;// = "export SRC_ITERS="+this.numberOfIterations;
            Process exec_process;// = Runtime.getRuntime().exec(cmd);

            // get filename for graph file
            Path p = Paths.get(applicationGraph);
            String graphFileName = p.getFileName().toString();
            String graphFileNameWithoutExt = graphFileName.substring(0, graphFileName.lastIndexOf("."));

            cmd = this.executableOfSimulation + " --systemoc-import-smx " + clusterDir.getCanonicalPath()+"/"+graphFileNameWithoutExt+"-synth-multicore-fsm-sched.smx"+" --systemoc-vpc-config "+clusterDir.getCanonicalPath()+"/config.xml";
            String[] envV = {"SRC_ITERS="+this.numberOfIterations};

            exec_process = Runtime.getRuntime().exec(cmd,envV,clusterDir.getAbsoluteFile());

        } catch (IOException e) {
            System.err.println("Got an exception during I/O with simulation:\n" + e);
        }

        objectiveValue = new DoubleValue(69.0);
        objectives.add(objective, objectiveValue);

        return implementation;
    }

    // Method that extracts the architecture graph and specifies the scheduler
    private Element processResources(Specification implementation) {
        // Define list of resources
        Element resources = new Element(RESOURCES);
        // Iterate the resources in the architecture graph
        for(Resource resource : implementation.getArchitecture().getVertices()) {
            String name = resource.getAttribute(NAMESTR);

            String scheduler = "RRNOPRE";
            Element component = new Element(COMPONENT);

            // set the name of the Resource
            component.setAttribute(NAMESTR, name);
            // set the scheduler
            component.setAttribute(SCHEDULER, scheduler);
            //  it is necessary to include attributes according to the scheduler
            resources.addContent(component);

            // Set the visualizator for the traces!

            Element tracing = new Element(ATTRIBUTE);
            tracing.setAttribute(TYPE, TRACING);
            tracing.setAttribute(VALUE, this.traceType.toString());

            component.addContent(tracing);

            Element sched = new Element(ATTRIBUTE);
            Element subElement = new Element(ATTRIBUTE);

            switch(this.schedulerType) {
                case TDMA:
                    // code block
                    break;
                /**
                 * Use FLEXRAY scheduler
                 */
                case FLEXRAY:
                    // code block
                    break;
                /**
                 * Use TTCC scheduler
                 */
                case TTCC:

                    break;
                /**
                 * Use Round Robin scheduler
                 */
                case RR:
                    sched.setAttribute(TYPE, SCHEDULER);

                    subElement.setAttribute(TYPE, "timeslice");
                    subElement.setAttribute(VALUE, Double.toString(this.timeSlice));

                    sched.addContent(subElement);
                    component.addContent(sched);
                    break;
                /**
                 * Use Round Robin no-preemption scheduler
                 */
                case RRNOPRE:
                    sched.setAttribute(TYPE, SCHEDULER);

                    subElement.setAttribute(TYPE, "fireActorInLoop");
                    if (this.fireActorInLoop)
                        subElement.setAttribute(VALUE, "true");
                    else
                        subElement.setAttribute(VALUE, "false");

                    sched.addContent(subElement);
                    component.addContent(sched);
                    break;
                /**
                 * Use Static Priority scheduler
                 */
                case SP:
                    break;
                /**
                 * Use Static Priority no-preemption scheduler
                 */
                case SPNOPRE:
                    break;
                /**
                 * Use First come first served scheduler
                 */
                case FCFS:
                    break;
                /**
                 * Use Rate monotonic scheduler
                 */
                case RM:
                    break;
                /**
                 * Use AVB scheduler
                 */
                case AVB:
                    break;
                /**
                 * Use MOST scheduler
                 */
                case MOST:
                    break;
                /**
                 * Use STREAMSHAPER scheduler
                 */
                case STREAMSHAPER:
                    break;
            }
        }

        return resources;
    }

    private Element processMappings(Specification implementation) {
        Element mappings = new Element("mappings");
        Application application = implementation.getApplication();

    //  Set<Cluster> clusters = implementation.getAttribute(DirectImplementationTransformer.TAG);
    //  for(Cluster clu : clusters) {
    //      //System.err.println("Clusters "+clu.getId());
    //      // Get all the clustered actors in each cluster
    //      Collection<Task> clusteredActors = clu.getClusteredActors();
    //      if (clusteredActors != null) {
    //          // Get the Resource in the Cluster
    //          Resource resource = null;
    //          for (Task clusteredActor: clusteredActors) {
    //              for(Mapping<Task,Resource> imapping : implementation.getMappings().get(clusteredActor)) {
    //                  resource = imapping.getTarget();
    //              }
    //          }
    //          // write cluster information into the VPC file!
    //          if (resource != null) {
    //           //   System.err.println("CLUSTER "+clu.getId()+ " MAPPED TO RESOURCE : "+resource.<String>getAttribute("name"));
    //           // create a mapping tag for each cluster
    //              Element mapping = new Element("mapping");
    //              mappings.addContent(mapping);
    //              mapping.setAttribute("source", "pgTop."+clu.getId());
    //              mapping.setAttribute("target", resource.<String>getAttribute("name"));
    //
    //              Element vpc_timing = new Element("timing");
    //              mapping.addContent(vpc_timing);
    //              vpc_timing.setAttribute("delay","0 ns");
    //          }
    //      }
    //
    //  }


        for (Task task : (Collection<Task>)application.getVertices()) {
          //if (task instanceof IFunction && !(systemoc.isFifo(task))) {
            if (!(task instanceof ICommunication)) {
                // found ACTOR: create mapping
                Collection<Mapping<Task,Resource>> binding = implementation.getMappings().get(task);
                // System.err.println(task.getName());
                for (Mapping<Task, Resource> imapping : binding) {
                    // Mapping m = (Mapping) imapping;
                    Resource resource = imapping.getTarget();

                    // create mapping tag for CMX file
                    Element mapping = new Element("mapping");
                    mappings.addContent(mapping);
                    if (task.<String>getAttribute("name") != null) {
                        mapping.setAttribute("source", task.<String>getAttribute("name"));
                    } else {
                        mapping.setAttribute("source", task.<String>getAttribute("name"));
                    }
                    mapping.setAttribute("target", resource.<String>getAttribute("name"));

                    //Read all the attributes inside each mapping
                    //Such attributes are the timing annotation
                    HashMap<String,Object> atts = imapping.getAttribute("vpc-actor-delay");
                    double timing = 0.0;

                    for (HashMap.Entry<String,Object> entry : atts.entrySet()) {
                        timing += (double)entry.getValue();
                    }

                    Element vpc_timing = new Element("timing");
                    mapping.addContent(vpc_timing);
                    // The specifaction contains times in seconds
                    String vpcDelay = (timing/ 1000000000) +" ns";
                    vpc_timing.setAttribute("delay", vpcDelay);

                }
            }

        }
        return mappings;
    }

    @Override
    public int getPriority() {
        // TODO Auto-generated method stub
        return 0;
    }

    private Element routingsToJdom( Routings<Task,Resource,Link> oneToOneRoutings, Specification implementation, Element resources) {
        Element topology = new Element(TOPOLOGY);
        topology.setAttribute("tracing", "true");

        Iterator<Task> iter = oneToOneRoutings.getTasks().iterator();

        // loop over messages
        while (iter.hasNext()) {
            Task message = iter.next();
            Iterator<Architecture<Resource,Link>> siter = oneToOneRoutings.getRoutings().iterator();

            // loop over every routing for a message (>1 if multicast)
            // Collection<IResource> hopsUsed = new LinkedList<IResource>();
            while (siter.hasNext()) {
                Architecture<Resource,Link> routing = siter.next();
      //        System.out.println("routing: " + routing.toString());
                routingToJdom(implementation, resources, topology, message, routing);
            }
        }
        return topology;
    }

    private void routingToJdom(Specification implementation, Element resources,Element topology, Task message, Architecture<Resource,Link> routing) {

        Collection<Task> preds = implementation.getApplication().getPredecessors(message);
        assert preds.size() == 1: "A message must only have one predecessor";
        Task predecessor = preds.iterator().next();

      //assert implementation.getRoutings().get(task) .getBindings().get(predecessor).size() == 1 : "Did not expect task to be bound to more than one resource";

        // find the right succ task
        for (Task successor : implementation.getApplication().getSuccessors(message)) {
      //    System.out.println("source " + task.toString());
            //assert implementation.getBindings().get(successor).size() == 1 : "Did not expect task to be bound to more than one resource";
      //    IResource target = implementation.getBindings().get(task).iterator().next().getTarget();
      //    System.out.println("target " + target.toString());

            String msgName = message.<String> getAttribute("NAME");
            // Depending on the way the model is read into DSE, a message can either be a FIFO
            // or an edge connecting an actor to FIFO. Please refer to SysteMoCXMLInputOutput for
            // details. If the name of the message ends with "_push" this should be the connection
            // from the actor to the FIFO, otherwise the message ends with "_pull" representing
            // the edge from the FIFO to the actor.

            if (msgName.endsWith("_push")) {
                Element pushRoute = new Element("route");
                topology.addContent(pushRoute);
                pushRoute.setAttribute("source", predecessor.<String> getAttribute("NAME"));
                pushRoute.setAttribute("destination", successor.<String> getAttribute("NAME"));
            } else if (msgName.endsWith("_pull")) {
                Element pullRoute = new Element("route");
                topology.addContent(pullRoute);
                pullRoute.setAttribute("source", predecessor.<String> getAttribute("NAME"));
                pullRoute.setAttribute("destination", successor.<String> getAttribute("NAME"));
            } else {
                // The message is a FIFO, create two route elements for VPC.
                Element pushRoute = new Element("route");
                topology.addContent(pushRoute);
                pushRoute.setAttribute("source", predecessor.<String> getAttribute("NAME"));
                pushRoute.setAttribute("destination", msgName);

                Element pullRoute = new Element("route");
                topology.addContent(pullRoute);
                pullRoute.setAttribute("source", msgName);
                pullRoute.setAttribute("destination", successor.<String> getAttribute("NAME"));
            }
        }
    }

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

