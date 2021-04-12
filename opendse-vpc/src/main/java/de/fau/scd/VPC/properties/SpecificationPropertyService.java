//-*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
//vim: set sw=4 ts=8 sts=4 et:
/*
* Copyright (c)
*   2021 FAU -- Nils Wilbert <nils.wilbert@fau.de>
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
package de.fau.scd.VPC.properties;

import java.util.Map;
import java.util.TreeMap;

import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Task;

public class SpecificationPropertyService {
    
    private SpecificationPropertyService() {
    }
    
    public static void setActorDelay(Mapping<Task, Resource>  mapping,  double delay) {
        Map<String, Double> delayMap = new TreeMap<String, Double>();
        delayMap.put("", delay);
        SpecificationPropertyService.setActorDelayMap(mapping, delayMap);;
    }

    public static double getActorDelay(Mapping<Task, Resource>  mapping) {
        Map<String, Double> mappingMap = SpecificationPropertyService.getActorDelayMap(mapping);   
        Double actorDelay = mappingMap.get("");
        assert actorDelay != null && mappingMap.size() == 1;
        return actorDelay;
    }
    
    public static void setActorDelayMap(Mapping<Task, Resource>  mapping,  Map<String, Double> delay) {
        mapping.setAttribute("vpc-actor-delay", delay);
    }

    public static Map<String, Double> getActorDelayMap(Mapping<Task, Resource>  mapping) {
        return mapping.<Map<String, Double>>getAttribute("vpc-actor-delay");
    }
}
