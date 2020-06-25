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

import java.util.Iterator;
import java.util.TreeMap;
import java.util.Map.Entry;

@SuppressWarnings("serial")
public class Environment extends TreeMap<String, String> {

    public Environment() {
        super();
    }

    public Environment(String encoding) {
        super();
        for (int start = 0; start < encoding.length();) {
            int index = encoding.indexOf(0, start);
            if (index == -1)
                index = encoding.length();
            String envVarDef = encoding.substring(start, index);
            if (!envVarDef.isEmpty()) {
                int indexAssign = envVarDef.indexOf('=');
                assert indexAssign != -1 : "Environment variable definition " + envVarDef + " must contain a '='!";
                String var   = envVarDef.substring(0, indexAssign);
                String value = envVarDef.substring(indexAssign+1);
                this.put(var, value);
            }
            start = index + 1;
        }
    }
    
    public String toString() {
        StringBuilder sb = new StringBuilder();

        Iterator<Entry<String, String>> i = entrySet().iterator();
        if (i.hasNext()) {
            for (;;) {
                Entry<String, String> e = i.next();
                sb.append(e.getKey());
                sb.append('=');
                sb.append(e.getValue());
                if (i.hasNext())
                    sb.append('\0');
                else
                    break;
            }
        }
        return sb.toString();
    }

}
