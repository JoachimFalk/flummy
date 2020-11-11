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
package de.fau.scd.SNG;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Set;

import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.SNG.SNGReader.SNGFormatErrorException;
import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mappings;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.SpecificationWrapper;
import net.sf.opendse.optimization.io.SpecificationTransformer;
import net.sf.opendse.optimization.io.SpecificationWrapperInstance;

public class SpecificationWrapperSNG implements SpecificationWrapper {

    final private SpecificationWrapperInstance specificationWrapperInstance;

    @Inject
    public SpecificationWrapperSNG(
            @Constant(namespace = SpecificationWrapperSNG.class, value = "sngFile") String sngFileName)
            throws IOException, FileNotFoundException, SNGFormatErrorException {
        SNGReader sngReader = new SNGReader(sngFileName);

        Architecture<Resource, Link> architecture = new Architecture<Resource, Link>();
        Mappings<Task, Resource> mappings = new Mappings<Task, Resource>();

        Application<Task, Dependency> application = sngReader.getApplication();

        Specification specification = new Specification(application, architecture, mappings);

        specificationWrapperInstance = new SpecificationWrapperInstance(specification);
    }

    @Override
    public Specification getSpecification() {
        return specificationWrapperInstance.getSpecification();
    }

    @Inject(optional = true)
    public void setSpecificationTransformers(Set<SpecificationTransformer> transformers) {
        specificationWrapperInstance.setSpecificationTransformers(transformers);
    }

}
