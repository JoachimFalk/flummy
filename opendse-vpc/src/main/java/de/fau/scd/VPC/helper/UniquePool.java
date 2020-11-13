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

import java.util.HashMap;
import java.util.Map;

public class UniquePool {

    public String createUniqeName(org.w3c.dom.Element eElement) {
        String name = eElement.getAttribute("name");
        // Fall back to id attribute.
        if (name == null)
            name = eElement.getAttribute("id");
        return createUniqeName(name, false);
    }

    public String createUniqeName() {
        return createUniqeName("__anonymous", true);
    }

    public String createUniqeName(String name, boolean numbered) {
        // Enforce unique names as names are used as IDs inside OpenDSE.
        Integer repeatEntry = knownNames.get(name);
        if (numbered && repeatEntry == null) {
            repeatEntry = 0;
        }
        if (repeatEntry != null) {
            String newName;
            int repeat = repeatEntry-1;
            do {
                repeat++;
                newName = name + "_" + Integer.toString(repeat);
            } while ((repeatEntry = knownNames.get(newName)) != null);
            knownNames.put(name, repeat);
            name = newName;
        }
        knownNames.put(name, 1);
        return name;
    }

    protected final Map<String, Integer> knownNames    = new HashMap<String, Integer>();
}
