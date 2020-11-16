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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.w3c.dom.Attr;

import de.fau.scd.VPC.helper.UniquePool;
import de.fau.scd.VPC.io.VPCConfigReader.VPCFormatErrorException;
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
      ) throws VPCFormatErrorException
    {
        this.uniquePool  = uniquePool;
        this.application = application;

        org.w3c.dom.Element eVPCConfig = vpcConfigReader.getDocumentElement();
        org.w3c.dom.Element eResources = VPCConfigReader.childElement(eVPCConfig, "resources");
        architecture = toArchitecture(eResources);
        org.w3c.dom.Element eMappings = VPCConfigReader.childElement(eVPCConfig, "mappings");
        mappings = toMappings(eMappings);
    }

    public Architecture<Resource, Link> getArchitecture() {
        return architecture;
    }

    public Mappings<Task, Resource> getMappings() {
        return mappings;
    }

    protected Architecture<Resource, Link> toArchitecture(org.w3c.dom.Element eResources
      ) throws VPCFormatErrorException
    {
        final Architecture<Resource, Link> architecture = new Architecture<Resource, Link>();

        for (org.w3c.dom.Element eComponent : SNGReader.childElements(eResources, "component")) {
            final String name = eComponent.getAttribute("name");
            final Resource resource = new Resource(name);

            if (resouceInstances.containsKey(name))
                throw new VPCFormatErrorException("Duplicate resouce \""+name+"\"!");
            resouceInstances.put(name, resource);
            architecture.addVertex(resource);
        }
        return architecture;
    }

    protected Mappings<Task, Resource> toMappings(org.w3c.dom.Element eResources
      ) throws VPCFormatErrorException
    {
        final Mappings<Task, Resource> mappings = new Mappings<Task, Resource>();

        for (org.w3c.dom.Element eMapping : VPCConfigReader.childElements(eResources, "mapping")) {
            final List<Task>     sources = new ArrayList<Task>();
            final List<Resource> targets = new ArrayList<Resource>();
            {
                final Attr    source         = eMapping.getAttributeNode("source");
                final Attr    sourceRegex    = eMapping.getAttributeNode("sourceRegex");
                final boolean sourceOptional = Boolean.valueOf(eMapping.getAttribute("sourceOptional"));

                if (source != null && sourceRegex != null) {
                    throw new VPCFormatErrorException("For mappings, source and sourceRegex must not both be defined!");
                } else if (source == null && sourceRegex == null) {
                    throw new VPCFormatErrorException("For mappings, either source or sourceRegex must be defined!");
                } else if (source != null) {
                    Task sourceTask = application.getVertex(source.getValue());
                    if (sourceTask == null && !sourceOptional)
                        throw new VPCFormatErrorException("Unknown source task \""+source+"\" in mapping!");
                    else
                        sources.add(sourceTask);
                } else {
                    Pattern regex = Pattern.compile(sourceRegex.getValue());
                    for (Task sourceTask : application.getVertices()) {
                        Matcher m = regex.matcher(sourceTask.getId());
                        if (m.find())
                            sources.add(sourceTask);
                    }
                    if (sources.isEmpty() && !sourceOptional)
                        throw new VPCFormatErrorException("Source regex \""+sourceRegex+"\" did non match any task in mapping!");
                }
            }
            {
                final Attr    target         = eMapping.getAttributeNode("target");
                final Attr    targetRegex    = eMapping.getAttributeNode("targetRegex");
                final boolean targetOptional = Boolean.valueOf(eMapping.getAttribute("targetOptional"));

                if (target != null && targetRegex != null) {
                    throw new VPCFormatErrorException("For mappings, source and sourceRegex must not both be defined!");
                } else if (target == null && targetRegex == null) {
                    throw new VPCFormatErrorException("For mappings, either source or sourceRegex must be defined!");
                } else if (target != null) {
                    Resource targetResource = architecture.getVertex(target.getValue());
                    if (targetResource == null && !targetOptional)
                        throw new VPCFormatErrorException("Unknown target resource \""+target+"\" in mapping!");
                    else
                        targets.add(targetResource);
                } else {
                    Pattern regex = Pattern.compile(targetRegex.getValue());
                    for (Resource targetResource : architecture.getVertices()) {
                        Matcher m = regex.matcher(targetResource.getId());
                        if (m.find())
                            targets.add(targetResource);
                    }
                    if (targets.isEmpty() && !targetOptional)
                        throw new VPCFormatErrorException("Regex regex \""+targetRegex+"\" did non match any resource in mapping!");
                }
            }
            for (Task source : sources) {
                for (Resource target : targets) {
                    final String name = uniquePool.createUniqeName(source.getId()+" -> "+target.getId(), false);
                    mappings.add(new Mapping<Task, Resource>(name, source, target));
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
