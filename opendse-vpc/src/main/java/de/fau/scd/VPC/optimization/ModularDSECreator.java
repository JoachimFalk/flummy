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

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

import org.opt4j.core.Genotype;
import org.opt4j.core.genotype.CompositeGenotype;
import org.opt4j.core.problem.Creator;

import com.google.inject.Inject;

import net.sf.opendse.model.Specification;
import net.sf.opendse.optimization.ParameterCreator;
import net.sf.opendse.optimization.SpecificationWrapper;

public class ModularDSECreator implements Creator<CompositeGenotype<String, Genotype>> {

    private Specification spec;
    private List<ImplementationCreatorDecoder> creatorDecoders;

    protected final ParameterCreator parameterCreator;

    @Inject
    public ModularDSECreator(Set<ImplementationCreatorDecoder> creatorDecoders,
            ParameterCreator     parameterCreator,
            SpecificationWrapper specWrapper)
    {
        super();
        this.parameterCreator = parameterCreator;
        this.spec = specWrapper.getSpecification();
        this.creatorDecoders = new ArrayList<ImplementationCreatorDecoder>(creatorDecoders);
        Collections.sort(this.creatorDecoders, new Comparator<ImplementationCreatorDecoder>() {
            @Override
            public int compare(ImplementationCreatorDecoder o1, ImplementationCreatorDecoder o2) {
                Integer i1 = o1.getPriority();
                Integer i2 = o2.getPriority();
                return i1.compareTo(i2);
            }
        });

    }

    @Override
    public CompositeGenotype<String, Genotype> create() {
        CompositeGenotype<String, Genotype> cg = new CompositeGenotype<String, Genotype>();
        cg.put("PARAMETER", parameterCreator.create());
        for (ImplementationCreatorDecoder creatorDecoder : creatorDecoders) {
            Genotype g = creatorDecoder.create(spec);
            if (g != null) {
                String name = creatorDecoder.getClass().getCanonicalName();
                cg.put(name, g);
            }
        }
        return cg;
    }

}
