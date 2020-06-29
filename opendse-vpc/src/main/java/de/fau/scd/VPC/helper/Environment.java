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
//      System.err.println("Encoding: "+encoding);
        for (int n = 0, m = encoding.length(); n < m;) {
            char c = '\0';
            String var = "";
            for (; n < m; ++n) {
                c = encoding.charAt(n);
                if (c == '=' || c == ';') {
                    break;
                } else if (c != '\\') {
                    var += c;
                } else {
                    ++n;
                    if (n >= m) {
                        throw new RuntimeException(
                            "Environemnt variable escape char '\\' must not be the last" +
                            " char in the evironment definition \""+encoding+"\"!");
                    }
                    char ce = encoding.charAt(n);
                    if (ce == '=') {
                        throw new RuntimeException(
                            "Environemnt variable escape char '\\' can not escape a '='!");
                    }
                    var += ce;
                }
            }
            if (c != '=') {
                throw new RuntimeException(
                    "Environment variable definition \"" + var + "\" must contain a '='!");
            }
            ++n; // Skip '='
//          System.err.println("Var: "+var+", encoding: "+encoding.substring(n));
            String value = "";
            for (; n < m; ++n) {
                c = encoding.charAt(n);
                if (c == ';') {
                    break;
                } else if (c != '\\') {
                    value += c;
                } else {
                    ++n;
                    if (n >= m) {
                        throw new RuntimeException(
                            "Environemnt variable escape char '\\' must not be the last" +
                            " char in the evironment definition \""+encoding+"\"!");
                    }
                    value += encoding.charAt(n);
                }
            }
            if (c == ';') {
                ++n; // Skip ';'
            }
//          System.err.println("Value: "+value+", encoding: "+encoding.substring(n));
            this.put(var, value);
        }
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();

        Iterator<Entry<String, String>> i = entrySet().iterator();
        if (i.hasNext()) {
            for (;;) {
                Entry<String, String> e = i.next();
                {
                    final String var   = e.getKey();
                    for(int n = 0, m = var.length() ; n < m ; ++n) {
                        char c = var.charAt(n);
                        if (c == '\\' || c == ';') {
                            sb.append('\\');
                            sb.append(c);
                        } else {
                            sb.append(c);
                        }
                    }
                }
                sb.append('=');
                {
                    final String value = e.getValue();
                    for(int n = 0, m = value.length() ; n < m ; ++n) {
                        char c = value.charAt(n);
                        if (c == '\\' || c == ';') {
                            sb.append('\\');
                            sb.append(c);
                        } else {
                            sb.append(c);
                        }
                    }
                }
                if (i.hasNext())
                    sb.append(';');
                else
                    break;
            }
        }
        return sb.toString();
    }

    public String [] getEnvironment() {
        String[] retval = new String[size()];

        int i = 0;
        for (Entry<String, String> e : entrySet()) {
            StringBuilder sb = new StringBuilder();
            sb.append(e.getKey());
            sb.append('=');
            sb.append(e.getValue());
            retval[i++] = sb.toString();
        }
        return retval;
    }

}
