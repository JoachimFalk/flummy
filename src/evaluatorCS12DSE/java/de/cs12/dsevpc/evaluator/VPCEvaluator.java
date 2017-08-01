package de.cs12.dsevpc.evaluator;

import static java.lang.annotation.RetentionPolicy.RUNTIME;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.annotation.Retention;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.StringTokenizer;

import org.jdom.Attribute;
import org.jdom.DocType;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import org.opt4j.core.Objective;
import org.opt4j.core.Objectives;
import org.opt4j.core.problem.Evaluator;

import com.google.inject.BindingAnnotation;
import com.google.inject.Inject;

import de.cs12.dse.model.IApplication;
import de.cs12.dse.model.IImplementation;
import de.cs12.dse.model.IMapping;
import de.cs12.dse.model.Routing;
import de.cs12.dse.model.components.Attributes;
import de.cs12.dse.model.components.ICommunication;
import de.cs12.dse.model.components.IFunction;
import de.cs12.dse.model.components.IResource;
import de.cs12.dse.model.components.ITask;
import de.cs12.dse.optimization.ImplementationWrapper;
import de.cs12.dsesgx.model.Properties;
import de.cs12.dsesgx.model.SgxModel;

public class VPCEvaluator implements Evaluator<ImplementationWrapper> {
  
  Objective SimTime = new Objective("SimTime[ns]", Objective.Sign.MIN);

  @Retention(RUNTIME)
  @BindingAnnotation
  public @interface StartScript {
  }

  
  static protected final String configFileName = "config.xml";

  static protected final String dtdFileName = "vpc.dtd";
  
  protected final String startscript;

  static protected final String VPCCONFIGURATION = "vpcconfiguration";

  static protected final String RESOURCES = "resources";

  static protected final String SCHEDULER = "scheduler";

  static protected final String COMPONENT = "component";

  static protected final String MAPPINGS = "mappings";
  
  static protected final String MAPPING = "mapping";  

  static protected final String PAJE = "PAJE";
  
  static protected final String DISTRIBUTION = "distribution";

  static protected final String TRACING = "tracing";

  static protected final String TYPE = "type";

  static protected final String VALUE = "value";

  static protected final String NAMESTR = "name";

  static protected final String ATTRIBUTE = "attribute";

  static protected final String TOPOLOGY = "topology";

  static protected final String PARAMETER = "parameter";

//  private static final String DISTRIBUTIONS = "distributions";

  protected final Properties properties;
  private final SgxModel systemoc;
  
  @Inject
  public VPCEvaluator(@StartScript String startscript, Properties properties, SgxModel smocModel) {
    this.properties = properties;
    this.systemoc = smocModel;
    this.startscript = startscript;
  }
  
  @Override
  public Objectives evaluate(ImplementationWrapper implementationWrapper) {
    IImplementation<ITask, IResource, IMapping> implementation = implementationWrapper.getImplementation();
    Objectives objectives = new Objectives();
    if (implementation == null || !implementation.isFeasible()) {
      objectives.add(SimTime, Objective.INFEASIBLE);
      return objectives;
    }
    Element root = new Element(VPCCONFIGURATION);
    Element resources = processResources(implementation);
    
    root.addContent(resources);
    root.addContent(processMappings(implementation));
    
    Map<ICommunication, List<Routing<IResource>>> oneToOneRoutings = extractRouting(implementation);
    root.addContent(routingsToJdom(oneToOneRoutings, implementation, resources));


    DocType vpcXml = new DocType(VPCCONFIGURATION, dtdFileName);
    Document doc = new Document(root, vpcXml);

    double simTime = Double.MAX_VALUE;
    
    try {
      // generate temporary directory
      File clusterDir = implementation.getTempDirectory();
      
      // generate VPC config
      File confFile = new File(clusterDir, configFileName);
      BufferedWriter bw = new BufferedWriter(new FileWriter(confFile));

      XMLOutputter serializer = new XMLOutputter(Format.getPrettyFormat());
      serializer.output(doc, bw);
      bw.close();
      
      // Measurement
      // generate new process running the bash-script
      String cmd = startscript + " " + clusterDir.getCanonicalPath();
      Process exec_process = Runtime.getRuntime().exec(cmd);
      BufferedReader is = new BufferedReader(new InputStreamReader(exec_process.getInputStream()));
      while (is.readLine() != null) {
              // needed under Windows XP if simulation produces some lines of
              // output
      }
      int returnCode = exec_process.waitFor();
      System.out.println("SIMULATION DONE!");
      if (returnCode != 0) {
              System.err.println("Simulation in directory " + clusterDir.getAbsolutePath()
                              + " exited with abnormal return code " + returnCode);

              throw new IllegalStateException("Simulation exited abnormally");
      }
      
   
      // read results
      File logFile = new File(clusterDir.getCanonicalPath(), "log");
      BufferedReader br = new BufferedReader(new FileReader(logFile));
      String line;
      String lastLine = null;
      while ((line = br.readLine()) != null) {
        lastLine = line;
      }
      if (lastLine.contains("overall simulated time:")){
              String[] elements = lastLine.split(" ");
              simTime = Double.parseDouble(elements[elements.length-2]);
      }
      br.close();
      logFile.delete();
      System.out.println("Simulation time: " + simTime +" ns");
      objectives.add(SimTime, simTime);
      
      
    } catch (IOException e) {
      System.err.println("Got an exception during I/O with simulation:\n" + e);
    } catch (InterruptedException e) {
      System.err.println("Got interupted while running simulation:\n" + e);
    }
    return objectives;

  }

  private Element processResources(IImplementation<ITask, IResource, IMapping> implementation) {  
    Element resources = new Element(RESOURCES);
    
    for (IResource resource : implementation.getAllocation().getVertices()) {
      String name = resource.getAttribute("NAME");
      String scheduler = "RoundRobin";
      Attribute xmlAttribute = resource.getAttribute("scheduler");
      if(xmlAttribute != null) {
        scheduler = xmlAttribute.getValue();
      }
 
      Element component = new Element(COMPONENT);
      component.setAttribute(NAMESTR, name);
  
      component.setAttribute(SCHEDULER, scheduler);
      resources.addContent(component);
  
      Element tracing = new Element(ATTRIBUTE);
      tracing.setAttribute(TYPE, TRACING);
      tracing.setAttribute(VALUE, PAJE);
      component.addContent(tracing);

    }
    return resources;
  }
  
  private Element processMappings(IImplementation<ITask, IResource, IMapping> implementation) {
    Element mappings = new Element("mappings");
    de.cs12.dse.model.IApplication<ITask> application = implementation.getApplication();

    for (ITask task : application.getVertices()) {
      if (task instanceof IFunction && !(systemoc.isFifo(task))) {
        // found ACTOR: create mapping
        Collection<IMapping> binding = implementation.getBindings().get(task);
        // System.err.println(task.getName());
        for (de.cs12.dse.model.IMapping imapping : binding) {
          // Mapping m = (Mapping) imapping;
          IResource resource = imapping.getTarget();

          // create mapping tag for CMX file
          Element mapping = new Element("mapping");
          mappings.addContent(mapping);
          mapping.setAttribute("source", task.<String>getAttribute("name"));
          mapping.setAttribute("target", resource.<String>getAttribute("NAME"));
          
          Attributes attributes = properties.getProperties(imapping);
          {
            Attributes attributes2 = imapping.getAttributes();
            for (String attribute : attributes2.keySet()) {
              System.out.println("map: " + attribute + " => " + imapping.getAttribute(attribute));
            }
          }
          {
            for (String attribute : attributes.keySet()) {
              System.out.println("prop: " + attribute + " => " + attributes.get(attribute));
            }
          }
          // Handle default timing
          {
            String vpcDelay = attributes.<String>get("delay");
            String vpcDII   = attributes.<String>get("dii");
            assert vpcDII == null || vpcDelay != null: "If a dii is specified, a delay must also be given!"; 

            if (vpcDelay != null) {
              Element timing = new Element("timing");
              mapping.addContent(timing);
              if (vpcDII != null) {
                timing.setAttribute("latency", vpcDelay);
                timing.setAttribute("dii", vpcDII);
              } else {
                timing.setAttribute("delay", vpcDelay);
              }
            }
          }
          // Handle action and guard timings
          {
            String vpcDelayCommaList = attributes.<String>get("vpc-function-delay");
            String vpcDIICommaList = attributes.<String>get("vpc-function-dii");
            
            HashMap<String, Element> delayMap = new HashMap<String, Element>();
            
            if (vpcDIICommaList != null) {
              StringTokenizer tokens = new StringTokenizer(vpcDIICommaList, " \t\n,");
              assert tokens.countTokens() % 2 != 0
                  : "The vpc-function-dii attribute must contain a list of comma separeted pairs of a function name and a DII timing, e.g., <fucnamea> <DIIa>, <fucnameb> <DIIb>, ...";
              // Iterate over comma list and process DII timings
              while (tokens.hasMoreTokens()) {
                String fname  = tokens.nextToken();
                String dii    = tokens.nextToken();
                Element timing = new Element("timing");
                timing.setAttribute("fname", fname);
                timing.setAttribute("dii", dii);
                assert !delayMap.containsKey(fname)
                    : "The vpc-function-dii attribute does not permit duplicate definitions of DII values for functions, i.e., "+fname+" is defined at least twice!";
                delayMap.put(fname, timing);
              }
            }
            if (vpcDelayCommaList != null) {
              StringTokenizer tokens = new StringTokenizer(vpcDelayCommaList, " \t\n,");
              assert tokens.countTokens() % 2 != 0
                  : "The vpc-function-delay attribute must contain a list of comma separeted pairs of a function name and a delay timing, e.g., <fucnamea> <delaya>, <fucnameb> <delayb>, ...";
              // Iterate over comma list and process delay timings
              while (tokens.hasMoreTokens()) {
                String fname  = tokens.nextToken();
                String delay  = tokens.nextToken();
                Element timing = delayMap.get(fname);
                if (timing == null) {
                  timing = new Element("timing");
                  timing.setAttribute("fname", fname);
                  delayMap.put(fname, timing);
                }
                mapping.addContent(timing);                
                String delayAttributeName = timing.getAttribute("dii") != null
                    ? "latency" : "delay";
                assert timing.getAttribute(delayAttributeName) == null
                    : "The vpc-function-delay attribute does not permit duplicate definitions of delay values for functions, i.e., "+fname+" is defined at least twice!";
                timing.setAttribute(delayAttributeName, delay);
              }
            }
            // Paranoia
            for (Map.Entry<String, Element> pair : delayMap.entrySet()) {
              assert pair.getValue().getAttribute("dii") == null || pair.getValue().getAttribute("latency") != null
                  : "The vpc-function-delay attribute must contain a delay value for a function that has a DII specified, i.e., "+pair.getKey()+" has a DII specified but no delay!"; 
            }
          }
        }

      } else if (systemoc.isFifo(task)) {
        // System.err.println("Ignore Channel: " +
        // ((Channel)task).toString()); // getName());
      }
    }
    return mappings;
  }
 
  private Element routingsToJdom(Map<ICommunication, List<Routing<IResource>>> oneToOneRoutings,
      IImplementation<ITask, IResource, IMapping> implementation, Element resources) {
    Element topology = new Element(TOPOLOGY);
    topology.setAttribute("tracing", "true");

    Iterator<ICommunication> iter = oneToOneRoutings.keySet().iterator();

    // loop over messages
    while (iter.hasNext()) {
      ICommunication message = iter.next();
      Iterator<Routing<IResource>> siter = oneToOneRoutings.get(message).iterator();
      // loop over every routing for a message (>1 if multicast)
      // Collection<IResource> hopsUsed = new LinkedList<IResource>();
      while (siter.hasNext()) {
        Routing<IResource> routing = siter.next();
//        System.out.println("routing: " + routing.toString());
        routingToJdom(implementation, resources, topology, message, routing);
      }
    }
    return topology;
  }

  private void routingToJdom(IImplementation<ITask, IResource, IMapping> implementation, Element resources,
      Element topology, ICommunication message, Routing<IResource> routing) {
   
    // find the right succ task
    Collection<ITask> succs = implementation.getApplication().getSuccessors(message);
    ITask successor = null;

    for (ITask task : succs) {
//      System.out.println("source " + task.toString());
      assert implementation.getBindings().get(task).size() == 1 : "Did not expect task to be bound to more than one resource";
//      IResource target = implementation.getBindings().get(task).iterator().next().getTarget();
//      System.out.println("target " + target.toString());
      ITask predecessor = (ITask) implementation.getApplication().getPredecessors(message).toArray()[0];
      
      successor = task;
      Element route1 = new Element("route");    
      topology.addContent(route1);      
      route1.setAttribute("source", predecessor.<String> getAttribute("NAME"));
      route1.setAttribute("destination", message.<String> getAttribute("NAME"));
      
      Element route2 = new Element("route");    
      topology.addContent(route2);
      route2.setAttribute("source", message.<String> getAttribute("NAME"));
      route2.setAttribute("destination", successor.<String> getAttribute("NAME"));

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

  private Collection<ITask> getReceivingFunctions(IImplementation<ITask, IResource, IMapping> implementation) {
    IApplication<ITask> application = implementation.getApplication();
    Collection<ITask> receivingFunctions = new ArrayList<ITask>();
    for (ITask task : application) {
      if (task instanceof IFunction) {
        if (application.getInEdges(task).size() > 0) {
          receivingFunctions.add(task);
        }
      }
    }
    return receivingFunctions;
  }


}
