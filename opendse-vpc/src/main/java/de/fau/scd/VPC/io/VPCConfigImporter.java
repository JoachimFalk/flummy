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

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.w3c.dom.Attr;

import de.fau.scd.VPC.io.Common.FormatErrorException;
import edu.uci.ics.jung.graph.util.EdgeType;
import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Mappings;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Task;

public class VPCConfigImporter {

    VPCConfigImporter(
        VPCConfigReader vpcConfigReader
      , UniquePool uniquePool
      , Application<Task, Dependency> application
      ) throws FormatErrorException
    {
        this.uniquePool  = uniquePool;
        this.application = application;

        org.w3c.dom.Element eVPCConfig = vpcConfigReader.getDocumentElement();
        org.w3c.dom.Element eResources = VPCConfigReader.childElement(eVPCConfig, "resources");
        org.w3c.dom.Element eLinks     = VPCConfigReader.childElement(eVPCConfig, "links");
        architecture = toArchitecture(eResources, eLinks);
        org.w3c.dom.Element eMappings = VPCConfigReader.childElement(eVPCConfig, "mappings");
        mappings = toMappings(eMappings);
    }

    public Architecture<Resource, Link> getArchitecture() {
        return architecture;
    }

    public Mappings<Task, Resource> getMappings() {
        return mappings;
    }

    protected Architecture<Resource, Link> toArchitecture(
        org.w3c.dom.Element eResources
      , org.w3c.dom.Element eLinks
      ) throws FormatErrorException
    {
        final Architecture<Resource, Link> architecture = new Architecture<Resource, Link>();

        for (org.w3c.dom.Element eComponent : VPCConfigReader.childElements(eResources, "component")) {
            final String name = eComponent.getAttribute("name");
            final Resource resource = new Resource(name);

            if (resouceInstances.containsKey(name))
                throw new FormatErrorException("Duplicate resouce \""+name+"\"!");
            resouceInstances.put(name, resource);
            AttributeHelper.addAttributes(eComponent, resource);
            architecture.addVertex(resource);
        }
        for (org.w3c.dom.Element eLink : VPCConfigReader.childElements(eLinks, "link")) {
            final String type = eLink.getAttribute("type");
            boolean directed = type.equals("DIRECTED");
            if (!type.isEmpty() && !type.equals("DIRECTED") && !type.equals("UNDIRECTED"))
                throw new FormatErrorException("Link type must either be DIRECTED or UNDIRECTED!");

            final List<Resource> sources = new ArrayList<Resource>();
            final List<Resource> targets = new ArrayList<Resource>();
            //String linkName;
            {
                final Attr    source         = eLink.getAttributeNode("source");
                final Attr    sourceRegex    = eLink.getAttributeNode("sourceRegex");

                if (source != null && sourceRegex != null) {
                    throw new FormatErrorException("For links, source and sourceRegex must not both be defined!");
                } else if (source == null && sourceRegex == null) {
                    throw new FormatErrorException("For links, either source or sourceRegex must be defined!");
                } else if (source != null) {
                    //linkName = source.getValue();
                    Resource sourceResource = architecture.getVertex(source.getValue());
                    if (sourceResource == null)
                        throw new FormatErrorException("Unknown source resource \""+source.getValue()+"\" in link!");
                    else
                        sources.add(sourceResource);
                } else {
                    //linkName = "regex:" + sourceRegex.getValue();
                    Pattern regex = Pattern.compile(sourceRegex.getValue());
                    for (Resource sourceResource : architecture.getVertices()) {
                        Matcher m = regex.matcher(sourceResource.getId());
                        if (m.find())
                            sources.add(sourceResource);
                    }
                    if (sources.isEmpty())
                        throw new FormatErrorException("Source regex \""+sourceRegex.getValue()+"\" did non match any resource in link!");
                }
            }
            //linkName += directed ? " -> " : " - ";
            {
                final Attr    target         = eLink.getAttributeNode("target");
                final Attr    targetRegex    = eLink.getAttributeNode("targetRegex");

                if (target != null && targetRegex != null) {
                    throw new FormatErrorException("For links, source and sourceRegex must not both be defined!");
                } else if (target == null && targetRegex == null) {
                    throw new FormatErrorException("For links, either source or sourceRegex must be defined!");
                } else if (target != null) {
                    //linkName += target.getValue();
                    Resource targetResource = architecture.getVertex(target.getValue());
                    if (targetResource == null)
                        throw new FormatErrorException("Unknown target resource \""+target.getValue()+"\" in link!");
                    else
                        targets.add(targetResource);
                } else {
                    //linkName += "regex:" + targetRegex.getValue();
                    Pattern regex = Pattern.compile(targetRegex.getValue());
                    for (Resource targetResource : architecture.getVertices()) {
                        Matcher m = regex.matcher(targetResource.getId());
                        if (m.find())
                            targets.add(targetResource);
                    }
                    if (targets.isEmpty())
                        throw new FormatErrorException("Regex regex \""+targetRegex.getValue()+"\" did non match any resource in link!");
                }
            }
            String connector = directed ? " -> " : " - ";
            for (Resource source : sources) {
                for (Resource target : targets) {
                    final String name = uniquePool.createUniqeName(source.getId()+connector+target.getId(), false);
                    final Link   link = new Link(name);
                    architecture.addEdge(link, source, target, directed ? EdgeType.DIRECTED : EdgeType.UNDIRECTED);
                    AttributeHelper.addAttributes(eLink, link);
                }
            }
        }
        return architecture;
    }

    protected Mappings<Task, Resource> toMappings(org.w3c.dom.Element eResources
      ) throws FormatErrorException
    {
        final Mappings<Task, Resource> mappings = new Mappings<Task, Resource>();

        final Pattern timeRegex = Pattern.compile("(-?[0-9]*(\\.[0-9]+)?([eE][-+]?[0-9]+)?) *(fs|ps|ns|us|ms|s|sec)");

        for (org.w3c.dom.Element eMapping : VPCConfigReader.childElements(eResources, "mapping")) {
            final List<Task>     sources = new ArrayList<Task>();
            final List<Resource> targets = new ArrayList<Resource>();
            String mappingName;
            {
                final Attr source         = eMapping.getAttributeNode("source");
                final Attr sourceRegex    = eMapping.getAttributeNode("sourceRegex");
                boolean    sourceOptional = false;
                {
                    final Attr attr = eMapping.getAttributeNode("sourceOptional");
                    if (attr != null)
                        try {
                            sourceOptional = (Boolean) AttributeHelper.toInstance(attr.getValue(), Boolean.class);
                        } catch (IllegalArgumentException | SecurityException | InstantiationException | IllegalAccessException
                                | InvocationTargetException | NoSuchMethodException e) {
                            throw new FormatErrorException("Mapping attribute sourceOptional must be a boolean not \""+attr.getValue()+"\"!");                        
                        }                    
                }
                if (source != null && sourceRegex != null) {
                    throw new FormatErrorException("For mappings, source and sourceRegex must not both be defined!");
                } else if (source == null && sourceRegex == null) {
                    throw new FormatErrorException("For mappings, either source or sourceRegex must be defined!");
                } else if (source != null) {
                    mappingName = source.getValue() + " -> ";
                    Task sourceTask = application.getVertex(source.getValue());
                    if (sourceTask == null && !sourceOptional)
                        throw new FormatErrorException("Unknown source task \""+source+"\" in mapping!");
                    else
                        sources.add(sourceTask);
                } else {
                    mappingName = "regex:" + sourceRegex.getValue() + " -> ";
                    Pattern regex = Pattern.compile(sourceRegex.getValue());
                    for (Task sourceTask : application.getVertices()) {
                        Matcher m = regex.matcher(sourceTask.getId());
                        if (m.find())
                            sources.add(sourceTask);
                    }
                    if (sources.isEmpty() && !sourceOptional)
                        throw new FormatErrorException("Source regex \""+sourceRegex+"\" did non match any task in mapping!");
                }
            }
            {
                final Attr target         = eMapping.getAttributeNode("target");
                final Attr targetRegex    = eMapping.getAttributeNode("targetRegex");
                boolean    targetOptional = false;
                {
                    final Attr attr = eMapping.getAttributeNode("targetOptional");
                    if (attr != null)
                        try {
                            targetOptional = (Boolean) AttributeHelper.toInstance(attr.getValue(), Boolean.class);
                        } catch (IllegalArgumentException | SecurityException | InstantiationException | IllegalAccessException
                                | InvocationTargetException | NoSuchMethodException e) {
                            throw new FormatErrorException("Mapping attribute targetOptional must be a boolean not \""+attr.getValue()+"\"!");                        
                        }                    
                }                
                if (target != null && targetRegex != null) {
                    throw new FormatErrorException("For mappings, source and sourceRegex must not both be defined!");
                } else if (target == null && targetRegex == null) {
                    throw new FormatErrorException("For mappings, either source or sourceRegex must be defined!");
                } else if (target != null) {
                    mappingName += target.getValue();
                    Resource targetResource = architecture.getVertex(target.getValue());
                    if (targetResource == null && !targetOptional)
                        throw new FormatErrorException("Unknown target resource \""+target+"\" in mapping!");
                    else
                        targets.add(targetResource);
                } else {
                    mappingName += "regex:" + targetRegex.getValue();
                    Pattern regex = Pattern.compile(targetRegex.getValue());
                    for (Resource targetResource : architecture.getVertices()) {
                        Matcher m = regex.matcher(targetResource.getId());
                        if (m.find())
                            targets.add(targetResource);
                    }
                    if (targets.isEmpty() && !targetOptional)
                        throw new FormatErrorException("Regex regex \""+targetRegex+"\" did non match any resource in mapping!");
                }
            }
            final Map<String, Object> vpcActorDelay = new HashMap<String, Object>();
            for (org.w3c.dom.Element eTiming : VPCConfigReader.childElements(eMapping, "timing")) {
                final String name  = eTiming.getAttribute("name");
                Object       value;
                final Attr   delay = eTiming.getAttributeNode("delay");
                if (delay != null) {
                    String delayText = delay.getValue();
                    Matcher m = timeRegex.matcher(delayText);
                    if (m.matches()) {
                        double v = Double.valueOf(m.group(1));
                        if (m.group(4).equals("fs")) {
                            v *= 1E-15;
                        } else if (m.group(4).equals("ps")) {
                            v *= 1E-12;
                        } else if (m.group(4).equals("ns")) {
                            v *= 1E-9;
                        } else if (m.group(4).equals("us")) {
                            v *= 1E-6;
                        } else if (m.group(4).equals("ms")) {
                            v *= 1E-3;
                        } else {
                            assert m.group(4).equals("s") || m.group(4).equals("sec")
                                : "Internal error in time unit handling!";
                        }
                        value = new Double(v);
                    } else
                        throw new FormatErrorException("Can parse delay value "+ delayText + " for timing "+name+" in mapping " + mappingName + "!");
                } else
                    throw new FormatErrorException("Missing delay value for timing "+name+" in mapping " + mappingName + "!");
                if (vpcActorDelay.containsKey(name))
                    throw new FormatErrorException("Dupicate timing "+name+" in mapping " + mappingName + "!");
                vpcActorDelay.put(name, value);
            }
            for (Task source : sources) {
                for (Resource target : targets) {
                    final String name = uniquePool.createUniqeName(source.getId()+" -> "+target.getId(), false);
                    final Mapping<Task, Resource> mapping = new Mapping<Task, Resource>(name, source, target);
                    if (!vpcActorDelay.isEmpty())
                        mapping.setAttribute("vpc-actor-delay", vpcActorDelay);
                    AttributeHelper.addAttributes(eMapping, mapping);
                    mappings.add(mapping);
                }
            }
        }
        return mappings;
    }

    protected final UniquePool uniquePool;

    protected final Application<Task, Dependency> application;

    protected final Map<String, Resource> resouceInstances = new HashMap<String, Resource>();

    protected final Architecture<Resource, Link> architecture;
    protected final Mappings<Task, Resource> mappings;
}
