// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2018 FAU -- Fedor Smirnov <fedor.smirnov@fau.de>
 *   2019 FAU -- Fedor Smirnov <fedor.smirnov@fau.de>
 *   2020 FAU -- Fedor Smirnov <fedor.smirnov@fau.de>
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

package de.fau.scd.VPC.decoders;

import java.util.*;

import org.opt4j.core.Genotype;
import org.opt4j.satdecoding.ContradictionException;

import com.google.inject.Inject;

import net.sf.opendse.model.*;
import net.sf.opendse.optimization.SATCreatorDecoder;
import net.sf.opendse.optimization.SpecificationWrapper;

import de.fau.scd.VPC.optimization.ImplementationCreatorDecoder;

public class SATDecoder implements ImplementationCreatorDecoder {

    protected final SATCreatorDecoder satDecoder;
    protected final Specification spec;

    @Inject
    public SATDecoder(SATCreatorDecoder satDecoder, SpecificationWrapper specWrapper) {
        super();
        this.satDecoder = satDecoder;
        this.spec = specWrapper.getSpecification();
    }

    @Override
    public Genotype create(Specification spec) {
        return satDecoder.create();
    }

    @Override
    public Specification decode(Specification implOld, Genotype satGenotype) {
//      System.err.println("SAT decoding [BEGIN]");
        Specification impl = null;
        try {
            impl = satDecoder.decode(satGenotype).getImplementation();
        } catch (ContradictionException e) {
            System.err.println("Stopping");
            throw e;
        }
//      System.err.println("SAT decoding [END]");
        // Fix function attributes
        Set<Function<Task, Dependency>> functions = impl.getApplication().getFunctions();
        for (Function<Task, Dependency> function : functions) {
            Task tImpl = function.iterator().next();
            Task tSpec = (Task) tImpl.getParent();
            Attributes attributes = spec.getApplication().getFunction(tSpec).getAttributes();
            for (Map.Entry<String, Object> e : attributes.entrySet()) {
                function.setAttribute(e.getKey(), e.getValue());
            }
        }
        return impl;
    }

    @Override
    public int getPriority() {
        return 0;
    }

}
