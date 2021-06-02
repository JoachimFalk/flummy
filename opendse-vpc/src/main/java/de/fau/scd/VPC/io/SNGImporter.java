// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
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
package de.fau.scd.VPC.io;

import java.util.*;

import org.apache.commons.collections15.multimap.MultiHashMap;

import de.fau.scd.VPC.io.Common.FormatErrorException;
import de.fau.scd.VPC.properties.ApplicationPropertyService;
import edu.uci.ics.jung.graph.util.EdgeType;

import net.sf.opendse.model.Application;
import net.sf.opendse.model.Attributes;
import net.sf.opendse.model.Communication;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.IAttributes;
import net.sf.opendse.model.Task;

/**
 * The {@code SNGImporter} converts a dataflow graph in SNG XML format given as
 * a {@code org.w3c.dom.Document} into an {@code Application}.
 *
 * @author Joachim Falk
 */
public class SNGImporter {

    public enum ChanTranslation {
        CHANS_ARE_DROPPED
      , CHANS_ARE_MEMORY_TASKS
    };

    public enum FIFOMerging {
        FIFOS_NO_MERGING
      , FIFOS_SAME_CONTENT_MERGING
      , FIFOS_SAME_PRODUCER_MERGING
    };

    SNGImporter(
        SNGReader          sngReader
      , UniquePool         uniquePool
      , ChanTranslation    chanTranslation
      , FIFOMerging        fifoMerging
      , boolean            generateMulticast
      ) throws FormatErrorException
    {
        this.uniquePool   = uniquePool;
        this.chanTranslat = chanTranslation;
        this.fifoMerging  = fifoMerging;
        this.genMulticast = generateMulticast;
        this.application  = toApplication(sngReader.getDocumentElement());
        {
            int actors   = 0;
            int channels = 0;
            int messages = 0;
            for (Task t : this.application) {
                switch (ApplicationPropertyService.getTaskType(t)) {
                    case EXE: ++actors; break;
                    case MEM: ++channels; break;
                    case MSG: ++messages; break;
                }
            }
            System.out.println("Loaded SNG file with "
                    + actors + " actors, "
                    + channels + " channels, and "
                    + messages + " messages!");
        }
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
        public final Task      exeTask;
        public final Map<String, Port> unboundPorts = new HashMap<String, Port>();

        ActorInstance(String name, ActorType type) {
            this.name = name;
            this.type = type;
            this.exeTask = new Task(name);
            unboundPorts.putAll(type.ports);
        }
    }

    static private class ChanInfo {
        public final String        name;
        public final int           tokenCapacity;
        public final int           initialTokens;
        public final int           tokenSize;
        public final ActorInstance sourceActor;
        public final String        sourcePort;
        public final ActorInstance targetActor;
        public final String        targetPort;
        public final Attributes    attrs;

        private ChanInfo(Map<String, ActorInstance> actorInstances
          , String     name
          , int        tokenCapacity
          , int        initialTokens
          , int        tokenSize
          , String     sourceActor
          , String     sourcePort
          , String     targetActor
          , String     targetPort
          , Attributes attrs) throws FormatErrorException
        {
            final ActorInstance sourceActorInstance = actorInstances.get(sourceActor);
            if (sourceActorInstance == null)
                throw new FormatErrorException("Unknown source actor instance \""+sourceActor+"\"!");
            final ActorInstance targetActorInstance = actorInstances.get(targetActor);
            if (targetActorInstance == null)
                throw new FormatErrorException("Unknown target actor instance \""+targetActor+"\"!");

            this.name = name;
            this.tokenCapacity = tokenCapacity;
            this.initialTokens = initialTokens;
            this.tokenSize = tokenSize;
            this.sourceActor = sourceActorInstance;
            this.sourcePort = sourcePort;
            this.targetActor = targetActorInstance;
            this.targetPort = targetPort;
            this.attrs = attrs;
            attrs.put(ApplicationPropertyService.attrTokenCapacity, tokenCapacity);
            attrs.put(ApplicationPropertyService.attrInitialToken, initialTokens);
            ApplicationPropertyService.setChannelSize(name, tokenSize);
        }

        static public ChanInfo forFIFO(
            Map<String, ActorInstance> actorInstances
          , org.w3c.dom.Element        eFifo
          ) throws FormatErrorException
        {
            final Attributes attrs = new Attributes();
            AttributeHelper.addAttributes(eFifo, attrs);
            final org.w3c.dom.Element eSource = SNGReader.childElement(eFifo, "source");
            final org.w3c.dom.Element eTarget = SNGReader.childElement(eFifo, "target");
            return new ChanInfo(actorInstances
              , eFifo.getAttribute("name")
              , Integer.valueOf(eFifo.getAttribute("size"))
              , Integer.valueOf(eFifo.getAttribute("initial"))
              , attrs.<Integer>getAttribute(ApplicationPropertyService.attrTokenSize)
              , eSource.getAttribute("actor")
              , eSource.getAttribute("port")
              , eTarget.getAttribute("actor")
              , eTarget.getAttribute("port")
              , attrs
              );
        }
    }

    private class ReadInstance {
        public ReadInstance(String msgName, ActorInstance target) {
            this.msgName = msgName;
            this.target  = target;
        }

        public void addChannel(ChanInfo chanInfo) {
            payload += chanInfo.tokenSize;
            representedChannels.add(chanInfo.name);
            String fromActor = chanInfo.targetActor.name+"."+chanInfo.targetPort;
            if (!mergingInfos.containsKey(fromActor)) 
                mergingInfos.put(fromActor, chanInfo.name);
            assert mergingInfos.get(fromActor).equals(chanInfo.name);
            
        }

        public void create(Task memTask, Application<Task, Dependency> app) {
            Communication readMsg = new Communication(msgName);
            app.addVertex(readMsg);
            {
                Dependency dependency = new Dependency(uniquePool.createUniqeName());
                app.addEdge(dependency, memTask, readMsg, EdgeType.DIRECTED);
            }
            {
                Dependency dependency = new Dependency(uniquePool.createUniqeName());
                app.addEdge(dependency, readMsg, target.exeTask, EdgeType.DIRECTED);
            }
            ApplicationPropertyService.setMessagePayload(readMsg, payload); 
            ApplicationPropertyService.setRepresentedReadChannels(readMsg, mergingInfos);         
        }

        private final String        msgName;
        private final ActorInstance target;
        private int                 payload = 0;
        private final Set<String>   representedChannels = new HashSet<>();
        // targetActor + targetPort to channel
        private final Map<String, String> mergingInfos = new HashMap<String, String>();

    }

    private class ChanInstance {
        public final Communication writeMsg;
        public final Task          memTask;

        public ChanInstance(String msgName) {
            this.writeMsg = new Communication(msgName);
            this.memTask  = null;
        }
        public ChanInstance(String msgName, String chanName) {
            this.writeMsg = new Communication(msgName);
            this.memTask  = new Task(chanName);
            ApplicationPropertyService.setTaskType(memTask,
                    ApplicationPropertyService.TaskType.MEM);
        }

        public void addChannel(ChanInfo chanInfo, String readMsgName) {           
            if (sourceActor == null)
                sourceActor = chanInfo.sourceActor;
            else
                assert sourceActor == chanInfo.sourceActor;
            if (storageSizes.get(chanInfo.sourcePort) == null)
                writeMsgPayload += chanInfo.tokenSize;
            
            String fromActor = chanInfo.sourceActor.name+"."+chanInfo.sourcePort;
            if (!mergingInfos.containsKey(fromActor)) {
                Set<String> mergedChannels = new HashSet<>();
                mergedChannels.add(chanInfo.name);
                mergingInfos.put(fromActor, mergedChannels);
            } else {
                Set<String> mergedChannels = mergingInfos.get(fromActor);
                mergedChannels.add(chanInfo.name);
                mergingInfos.put(fromActor, mergedChannels);
            }
            // We unify data storage for all writes from the same port, i.e.,
            // channels into which identical data is written.
            {
                Integer size = storageSizes.get(chanInfo.sourcePort);
                if (size == null ||
                    size < chanInfo.tokenCapacity * chanInfo.tokenSize)
                    storageSizes.put(chanInfo.sourcePort,
                            chanInfo.tokenCapacity * chanInfo.tokenSize);
            }
            ReadInstance readInstance = readInstances.get(readMsgName);
            if (readInstance == null) {
                readInstance = new ReadInstance(readMsgName, chanInfo.targetActor);
                readInstances.put(readMsgName, readInstance);
            }
            readInstance.addChannel(chanInfo);
            for (Map.Entry<String, Object> attr : chanInfo.attrs.entrySet()) {
                if (attrsInconsistent.contains(attr.getKey()))
                        continue;
                Object oldValue = attrs.put(attr.getKey(), attr.getValue());
                if (oldValue != null) {
                    boolean inconsistent = false;
                    // Check for consistency
                    if (!oldValue.getClass().equals(attr.getValue().getClass()))
                        inconsistent = true;
                    else
                        inconsistent = !oldValue.equals(attr.getValue());
                    if (inconsistent) {
                        attrsInconsistent.add(attr.getKey());
                        attrs.remove(attr.getKey());
                    }
                }
            }
        }

        public void create(Application<Task, Dependency> app) {
            app.addVertex(memTask);
            {
                int storageSize = 0;
                for (Integer size : storageSizes.values())
                    storageSize += size;
                ApplicationPropertyService.setStorageSize(memTask, storageSize);
            }
            for (Map.Entry<String, Object> attr : attrs.entrySet()) {
                memTask.setAttribute(attr.getKey(), attr.getValue());
            }
            app.addVertex(writeMsg);
            ApplicationPropertyService.setMessagePayload(writeMsg, writeMsgPayload);
            ApplicationPropertyService.setRepresentedWriteChannels(writeMsg, mergingInfos);
            {
                Dependency dependency = new Dependency(uniquePool.createUniqeName());
                app.addEdge(dependency, sourceActor.exeTask, writeMsg, EdgeType.DIRECTED);
            }
            {
                Dependency dependency = new Dependency(uniquePool.createUniqeName());
                app.addEdge(dependency, writeMsg, memTask, EdgeType.DIRECTED);
            }
            for (ReadInstance readInstance : readInstances.values()) {
                readInstance.create(memTask, app);
            }
        }

        private ActorInstance sourceActor = null;
        // Unification of all attributes from the represented channels
        private final Attributes  attrs = new Attributes();
        // Attribute names that have an inconsistent value and, thus, are not contained in attrs.
        private final Set<String> attrsInconsistent = new HashSet<>();
        private final Map<String, ReadInstance> readInstances = new HashMap<>();
        private final Map<String, Integer>      storageSizes = new HashMap<>();
        // sourceActor + sourcePort to channels
        private final Map<String, Set<String>> mergingInfos = new HashMap<>();
        private int writeMsgPayload = 0;
    }

    /**
     * Convert a specification XML element to an application
     *
     * @param eNetworkGraph the networkGraph XML element
     * @return the application
     * @throws SNGFormatErrorException
     */
    protected Application<Task, Dependency> toApplication(org.w3c.dom.Element eNetworkGraph) throws FormatErrorException {
        Application<Task, Dependency> app = new Application<Task, Dependency>();

        final Map<String, ActorType>     actorTypes     = new HashMap<String, ActorType>();
        final Map<String, ActorInstance> actorInstances = new HashMap<String, ActorInstance>();

        for (org.w3c.dom.Element eActorType : SNGReader.childElements(eNetworkGraph, "actorType")) {
            final ActorType actorType = toActorType(eActorType);
            if (actorTypes.containsKey(actorType.name))
                throw new FormatErrorException("Duplicate actor type \""+actorType.name+"\"!");
            actorTypes.put(actorType.name, actorType);
        }
        // Translate actorInstances
        for (org.w3c.dom.Element eActorInstance : SNGReader.childElements(eNetworkGraph, "actorInstance")) {
            final ActorInstance actorInstance = toActorInstance(eActorInstance, actorTypes);
            if (actorInstances.containsKey(actorInstance.name))
                throw new FormatErrorException("Duplicate actor instance \""+actorInstance.name+"\"!");
            actorInstances.put(actorInstance.name, actorInstance);
            if (chanTranslat == ChanTranslation.CHANS_ARE_MEMORY_TASKS)
                ApplicationPropertyService.setTaskType(actorInstance.exeTask,
                        ApplicationPropertyService.TaskType.EXE);
            app.addVertex(actorInstance.exeTask);
        }
        // Translate FIFOs
        translateFIFOs(eNetworkGraph, app, actorInstances);
        // Translate registers
        translateRegisters(eNetworkGraph, app, actorInstances);
        return app;
    }

    protected void translateFIFOs(
        org.w3c.dom.Element           eNetworkGraph
      , Application<Task, Dependency> app
      , Map<String, ActorInstance>    actorInstances
      ) throws FormatErrorException
    {
        switch (chanTranslat) {
        case CHANS_ARE_DROPPED: {
            final Map<String, Task> msgInstances = new HashMap<String, Task>();

            for (org.w3c.dom.Element eFifo : SNGReader.childElements(eNetworkGraph, "fifo")) {
                ChanInfo chanInfo = ChanInfo.forFIFO(actorInstances, eFifo);
                String msgName = chanInfo.sourceActor.name+"."+chanInfo.sourcePort;
                if (!genMulticast)
                    msgName = uniquePool.createUniqeName(msgName, true);
                Task msgTask = msgInstances.get(msgName);
                if (msgTask == null) {
                    msgTask = new Communication(msgName);
                    msgInstances.put(msgName, msgTask);
                    IAttributes attrs = new Attributes();
                    AttributeHelper.addAttributes(eFifo, attrs);
                    int tokenSize = attrs.<Integer>getAttribute(ApplicationPropertyService.attrTokenSize);
                    ApplicationPropertyService.setMessagePayload(msgTask, tokenSize);
                    app.addVertex(msgTask);
                    {
                        Dependency dependency = new Dependency(uniquePool.createUniqeName());
                        app.addEdge(dependency, chanInfo.sourceActor.exeTask, msgTask, EdgeType.DIRECTED);
                    }
                }
                {
                    Dependency dependency = new Dependency(uniquePool.createUniqeName());
                    app.addEdge(dependency, msgTask, chanInfo.targetActor.exeTask, EdgeType.DIRECTED);
                }
            }
            break;
        }
        case CHANS_ARE_MEMORY_TASKS: {
            final Map<String, ChanInstance>  chanInstances  = new HashMap<String, ChanInstance>();
            for (org.w3c.dom.Element eFifo : SNGReader.childElements(eNetworkGraph, "fifo")) {
                ChanInfo chanInfo = ChanInfo.forFIFO(actorInstances, eFifo);
                String writeMsgName = null;
                
                switch (fifoMerging) {
                case FIFOS_NO_MERGING:
                    writeMsgName = uniquePool.createUniqeName(
                        chanInfo.sourceActor.name+"."+chanInfo.sourcePort, true);
                    break;
                case FIFOS_SAME_CONTENT_MERGING:
                    writeMsgName = chanInfo.sourceActor.name+"."+chanInfo.sourcePort;
                    break;
                case FIFOS_SAME_PRODUCER_MERGING:
                    writeMsgName = chanInfo.sourceActor.name+".out";               
                    break;
                }
                ChanInstance chanInstance = chanInstances.get(writeMsgName);
                if (chanInstance == null) {
                    switch (fifoMerging) {
                    case FIFOS_NO_MERGING:
                        chanInstance = new ChanInstance(writeMsgName, chanInfo.name);
                        break;
                    default:
                        chanInstance = new ChanInstance(writeMsgName, "cf:"+writeMsgName);
                        break;
                    }
                    chanInstances.put(writeMsgName, chanInstance);
                }
                if (fifoMerging != FIFOMerging.FIFOS_NO_MERGING)
                    ApplicationPropertyService.addRepresentedChannel(chanInstance.memTask, chanInfo.name);
                String readMsgName = null;
                switch (fifoMerging) {
                case FIFOS_NO_MERGING:
                case FIFOS_SAME_CONTENT_MERGING:
                    readMsgName = chanInfo.targetActor.name+"."+chanInfo.targetPort;
                    break;
                case FIFOS_SAME_PRODUCER_MERGING:
                    readMsgName = chanInfo.targetActor.name+"."+chanInfo.sourceActor.name;
                    break;
                }
                chanInstance.addChannel(chanInfo, readMsgName);
            }
            for (ChanInstance chanInstance : chanInstances.values())
                chanInstance.create(app);
            break;
        }
        }
    }

    protected void translateRegisters(
        org.w3c.dom.Element           eNetworkGraph
      , Application<Task, Dependency> app
      , Map<String, ActorInstance>    actorInstances
      ) throws FormatErrorException
    {
        switch (chanTranslat) {
        case CHANS_ARE_DROPPED: {
            final Map<String, Task> msgInstances = new HashMap<String, Task>();

            for (org.w3c.dom.Element eRegister : SNGReader.childElements(eNetworkGraph, "register")) {
                if (!SNGReader.childElements(eRegister, "target").iterator().hasNext())
                    continue;

                for (org.w3c.dom.Element eSource : SNGReader.childElements(eRegister, "source")) {
                    final String sourceActor = eSource.getAttribute("actor");
                    final String sourcePort  = eSource.getAttribute("port");
                    final ActorInstance sourceActorInstance = actorInstances.get(sourceActor);
                    if (sourceActorInstance == null)
                        throw new FormatErrorException("Unknown source actor instance \""+sourceActor+"\"!");

                    String msgName = sourceActor+"."+sourcePort;
                    if (!genMulticast)
                        msgName = uniquePool.createUniqeName(msgName, true);
                    Task msgTask = msgInstances.get(msgName);
                    if (msgTask == null) {
                        msgTask = new Communication(msgName);
                        msgInstances.put(msgName, msgTask);
                        AttributeHelper.addAttributes(eRegister, msgTask);
                        int tokenSize = ApplicationPropertyService.getTokenSize(msgTask);
                        ApplicationPropertyService.setMessagePayload(msgTask, tokenSize);
                        app.addVertex(msgTask);
                        {
                            Dependency dependency = new Dependency(uniquePool.createUniqeName());
                            app.addEdge(dependency, sourceActorInstance.exeTask, msgTask, EdgeType.DIRECTED);
                        }
                    }
                    for (org.w3c.dom.Element eTarget : SNGReader.childElements(eRegister, "target")) {
                        final String targetActor = eTarget.getAttribute("actor");
                        final String targetPort  = eTarget.getAttribute("port");
                        final ActorInstance targetActorInstance = actorInstances.get(targetActor);
                        if (targetActorInstance == null)
                            throw new FormatErrorException("Unknown target actor instance \""+targetActorInstance+"\"!");

                        Dependency dependency = new Dependency(uniquePool.createUniqeName());
                        app.addEdge(dependency, msgTask, targetActorInstance.exeTask, EdgeType.DIRECTED);
                    }
                }
            }
            break;
        }
        case CHANS_ARE_MEMORY_TASKS: {
            final Map<String, ChanInstance>  chanInstances  = new HashMap<String, ChanInstance>();

            for (org.w3c.dom.Element eRegister : SNGReader.childElements(eNetworkGraph, "register")) {
                String name  = eRegister.getAttribute("name");
                
                Task memTask = new Task(name);
                ApplicationPropertyService.setTaskType(memTask,
                        ApplicationPropertyService.TaskType.MEM);
                ApplicationPropertyService.setTokenCapacity(memTask, 1);
                AttributeHelper.addAttributes(eRegister, memTask);
                int tokenSize = ApplicationPropertyService.getTokenSize(memTask);
                ApplicationPropertyService.setStorageSize(memTask, tokenSize);

                for (org.w3c.dom.Element eSource : SNGReader.childElements(eRegister, "source")) {
                    final String sourceActor = eSource.getAttribute("actor");
                    final String sourcePort  = eSource.getAttribute("port");
                    final ActorInstance sourceActorInstance = actorInstances.get(sourceActor);
                    if (sourceActorInstance == null)
                        throw new FormatErrorException("Unknown source actor instance \""+sourceActor+"\"!");

                    String messageName = sourceActor+"."+sourcePort;
                    if (fifoMerging != FIFOMerging.FIFOS_NO_MERGING)
                        messageName = uniquePool.createUniqeName(messageName, true);
                    ChanInstance chanInstance = chanInstances.get(messageName);
                    if (chanInstance == null) {
                        chanInstance = new ChanInstance(messageName);
                        chanInstances.put(messageName, chanInstance);
                        ApplicationPropertyService.setMessagePayload(chanInstance.writeMsg, tokenSize);
                        app.addVertex(chanInstance.writeMsg);
                        {
                            Dependency dependency = new Dependency(uniquePool.createUniqeName());
                            app.addEdge(dependency, sourceActorInstance.exeTask, chanInstance.writeMsg, EdgeType.DIRECTED);
                        }
                    }
                    {
                        Dependency dependency = new Dependency(uniquePool.createUniqeName());
                        app.addEdge(dependency, chanInstance.writeMsg, memTask, EdgeType.DIRECTED);
                    }
                }
                for (org.w3c.dom.Element eTarget : SNGReader.childElements(eRegister, "target")) {
                    final String targetActor = eTarget.getAttribute("actor");
                    final String targetPort  = eTarget.getAttribute("port");
                    final ActorInstance targetActorInstance = actorInstances.get(targetActor);
                    if (targetActorInstance == null)
                        throw new FormatErrorException("Unknown target actor instance \""+targetActorInstance+"\"!");

                    Communication readMsg = new Communication(targetActor+"."+targetPort);
                    ApplicationPropertyService.setMessagePayload(readMsg, tokenSize);
                    app.addVertex(readMsg);
                    {
                        Dependency dependency = new Dependency(uniquePool.createUniqeName());
                        app.addEdge(dependency, memTask, readMsg, EdgeType.DIRECTED);
                    }
                    {
                        Dependency dependency = new Dependency(uniquePool.createUniqeName());
                        app.addEdge(dependency, readMsg, targetActorInstance.exeTask, EdgeType.DIRECTED);
                    }
                }
            }
//          for (ChanInstance chanInstance : chanInstances.values())
//              chanInstance.create(app);
            break;
        }
        }
    }

    /**
     * Convert a actorType XML element to an ActorType
     *
     * @param eActorType the actorType XML element
     * @return an ActorType
     * @throws SNGFormatErrorException
     */
    protected ActorType toActorType(org.w3c.dom.Element eActorType) throws FormatErrorException {
        final String    actorTypeName = eActorType.getAttribute("name");
        final ActorType actorType     = new ActorType(actorTypeName);

        for (org.w3c.dom.Element ePort : SNGReader.childElements(eActorType, "actorType")) {
            String name = ePort.getAttribute("name");
            String type = ePort.getAttribute("type");
            Port.Direction d = Port.Direction.valueOf(type.toUpperCase());
            if (actorType.ports.containsKey(name))
                throw new FormatErrorException("Duplicate actor port \""+name+"\" in actor type \""+actorTypeName+"\"!");
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
            throws FormatErrorException
    {
        final String type = eActorInstance.getAttribute("type");
        final String name = eActorInstance.getAttribute("name");
        final ActorType actorType = actorTypes.get(type);
        if (actorType == null)
            throw new FormatErrorException("Unknown actor type \""+type+"\" for actor instance \""+name+"\"!");
        return new ActorInstance(name, actorType);
    }

    protected final UniquePool      uniquePool;
    protected final ChanTranslation chanTranslat;
    protected final FIFOMerging     fifoMerging;
    protected final boolean         genMulticast;

    protected final Application<Task, Dependency> application;
}
