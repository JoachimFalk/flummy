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

package de.fau.scd.VPC.io;

import org.opt4j.core.config.annotations.Info;
//import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Panel;
//import org.opt4j.core.config.annotations.Required;

import de.fau.scd.VPC.config.properties.AttributeAnnotations;
import de.fau.scd.VPC.config.visualization.PropertyPanel;
import net.sf.opendse.optimization.io.IOModule;

@Panel(value = PropertyPanel.class)
public class AttributeAnnotatorModule extends IOModule {
    
    @SuppressWarnings("serial")
    protected static class AttributeAnnotationsImpl
    extends
        AttributeAnnotations
    implements
        AttributeAnnotator.AttributeAnnotations
    {
    }
    @Info("Additional attributes to annotated to the specification")
//  @Order(3)
//  @Required(property = "dfgSource", elements = { "DFG_FROM_SIM_EXPORT" })
    protected final AttributeAnnotationsImpl attributeAnnotations = new AttributeAnnotationsImpl();

    public AttributeAnnotations getAttributeAnnotations() {
        return attributeAnnotations;
    }

    public void setAttributeAnnotations(AttributeAnnotations ann) {
        this.attributeAnnotations.assign(ann);
    }

    @Override
    protected void config() {
        bindSpecificationTransformer(AttributeAnnotator.class);
        bind(AttributeAnnotator.AttributeAnnotations.class).toInstance(attributeAnnotations);
        
    }

}
