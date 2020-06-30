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

package de.fau.scd.VPC.helper;

import java.awt.Component;

import org.opt4j.core.config.Property;
import org.opt4j.core.config.PropertyModule;
import org.opt4j.core.config.visualization.FileChooser;
import org.opt4j.core.config.visualization.Format;

@SuppressWarnings("serial")
public class PropertyPanel extends org.opt4j.core.config.visualization.PropertyPanel {

    public PropertyPanel(PropertyModule module, FileChooser fileChooser, Format format) {
        super(module, fileChooser, format);
    }

    protected Component createComponent(final Property property) {
        Class<?> type = property.getType();

        if (type.isAssignableFrom(Environment.class)) {
            return new EnvironmentPanel(property);
        } else if (type.isAssignableFrom(Objectives.class)) {
            return new ObjectivesPanel(property);
        } else {
            return super.createComponent(property);
        }
    }

}