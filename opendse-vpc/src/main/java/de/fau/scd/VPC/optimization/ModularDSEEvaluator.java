// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
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

package de.fau.scd.VPC.optimization;

import java.util.Map;
import java.util.Set;

import org.opt4j.core.Objectives;
import org.opt4j.core.start.Constant;

import com.google.inject.Provider;
import com.google.inject.Inject;

import net.sf.opendse.model.*;
import net.sf.opendse.optimization.DesignSpaceExplorationEvaluator;
import net.sf.opendse.optimization.ImplementationEvaluator;
import net.sf.opendse.optimization.ImplementationWrapper;

import de.fau.scd.VPC.properties.ImplementationPropertyService;

public class ModularDSEEvaluator extends DesignSpaceExplorationEvaluator {

    public enum DiscardImplementations {
        DISCARD_NO_IMPLEMENTATIONS,
        DISCARD_INFEASIBLE_IMPLEMENTATIONS,
        DISCARD_ALL_IMPLEMENTATIONS
    }

    @Inject
    public ModularDSEEvaluator(
        Set<ImplementationEvaluator> evaluators
      , Provider<Objectives> objectivesProvider
      , @Constant(namespace = ModularDSEEvaluator.class, value = "discardImplementations")
        DiscardImplementations discardImplementations)
    {
        super(evaluators, objectivesProvider);
        this.discardImplementations = discardImplementations;
    }

    @Override
    public Objectives evaluate(ImplementationWrapper wrapper) {
        Objectives objs = super.evaluate(wrapper);

        boolean discard = false;

        switch (discardImplementations) {
            case DISCARD_ALL_IMPLEMENTATIONS:
                discard = true;
                break;
            case DISCARD_INFEASIBLE_IMPLEMENTATIONS:
                discard = ImplementationPropertyService
                    .isInfeasible(wrapper.getImplementation(), objs);
                break;
            case DISCARD_NO_IMPLEMENTATIONS:
                discard = false;
                break;
        }

        if (discard) {
            Attributes attrs = wrapper.getImplementation().getAttributes();
            Specification emptyImpl = new Specification(emptyApp, emptyArch, emptyMaps);
            for (Map.Entry<String, Object> e : attrs.entrySet())
                emptyImpl.setAttribute(e.getKey(), e.getValue());
            wrapper.setImplementation(emptyImpl);
        }

        return objs;
    }

    private final DiscardImplementations discardImplementations;

    private static final Application<Task, Dependency> emptyApp  = new Application<>();
    private static final Architecture<Resource, Link>  emptyArch = new Architecture<>();
    private static final Mappings<Task, Resource>      emptyMaps = new Mappings<>();

}
