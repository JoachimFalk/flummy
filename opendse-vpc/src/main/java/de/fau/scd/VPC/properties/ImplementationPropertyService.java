//-*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
//vim: set sw=4 ts=8 sts=4 et:
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
package de.fau.scd.VPC.properties;

import org.opt4j.core.Objectives;
import org.opt4j.core.Objective;
import org.opt4j.core.Value;

import net.sf.opendse.model.Specification;

public class ImplementationPropertyService {

    private ImplementationPropertyService() {}

    /// Attribute name used to store infeasible information of an implementation.
    static public final String attrInfeasible = "INFEASIBLE";

    /**
     * Check if the implementation has been marked infeasible.
     *
     * @param impl
     * @return feasible
     */
    public static boolean isInfeasible(Specification impl, Objectives objectives) {
        Boolean infeasible = impl.<Boolean>getAttribute(attrInfeasible);
        if (infeasible == null) {
            infeasible = false;
            for (Value<?> value : objectives.getValues()) {
                if (value == Objective.INFEASIBLE || value == null || value.getValue() == null) {
                    infeasible = true;
                    break;
                }
            }
        }
        return infeasible;
    }

    /**
     * Mark the implementation as infeasible.
     *
     * @param impl
     */
    public static void setInfeasible(Specification impl) {
        impl.setAttribute(attrInfeasible, true);
    }

}
