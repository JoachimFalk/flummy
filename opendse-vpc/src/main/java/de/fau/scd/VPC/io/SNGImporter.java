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

import java.util.HashMap;
import java.util.Map;

import de.fau.scd.VPC.io.Common.FormatErrorException;

import edu.uci.ics.jung.graph.util.EdgeType;

import net.sf.opendse.model.Application;
//import net.sf.opendse.model.Attributes;
import net.sf.opendse.model.Communication;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Task;

/**
 * The {@code SNGImporter} converts a dataflow graph in SNG XML format given as
 * a {@code org.w3c.dom.Document} into an {@code Application}.
 *
 * @author Joachim Falk
 */
public class SNGImporter {


    SNGImporter(SNGReader sngReader, UniquePool uniquePool) throws FormatErrorException
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
    protected Application<Task, Dependency> toApplication(org.w3c.dom.Element eNetworkGraph) throws FormatErrorException {
        Application<Task, Dependency> application = new Application<Task, Dependency>();

        final Map<String, ActorType>     actorTypes     = new HashMap<String, ActorType>();
        final Map<String, ActorInstance> actorInstances = new HashMap<String, ActorInstance>();

        for (org.w3c.dom.Element eActorType : SNGReader.childElements(eNetworkGraph, "actorType")) {
            final ActorType actorType = toActorType(eActorType);
            if (actorTypes.containsKey(actorType.name))
                throw new FormatErrorException("Duplicate actor type \""+actorType.name+"\"!");
            actorTypes.put(actorType.name, actorType);
        }
        for (org.w3c.dom.Element eActorInstance : SNGReader.childElements(eNetworkGraph, "actorInstance")) {
            final ActorInstance actorInstance = toActorInstance(eActorInstance, actorTypes);
            if (actorInstances.containsKey(actorInstance.name))
                throw new FormatErrorException("Duplicate actor instance \""+actorInstance.name+"\"!");
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
                throw new FormatErrorException("Unknown source actor instance \""+sourceActor+"\"!");

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
                throw new FormatErrorException("Unknown target actor instance \""+targetActorInstance+"\"!");
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
        final Task task = new Task(name);
        return new ActorInstance(name, actorType, task);
    }

    protected final UniquePool uniquePool;

    protected final Application<Task, Dependency> application;
}
