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

package de.fau.scd.VPC.config.properties;

import java.util.Iterator;
import java.util.PrimitiveIterator;
import java.util.TreeMap;
import java.util.Map.Entry;

@SuppressWarnings("serial")
public class Environment extends TreeMap<String, String> {

    public Environment() {
        super();
    }

    public Environment(String encoding) {
        super();
        assign(encoding);
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();

        Iterator<Entry<String, String>> i = entrySet().iterator();
        while (i.hasNext()) {
            Entry<String, String> e = i.next();
            enQuote(sb, e.getKey());
            sb.append('=');
            enQuote(sb, e.getValue());
            if (i.hasNext())
                sb.append(';');
        }
        return sb.toString();
    }

    public void assign(Environment env) {
        if (env != this) {
            clear(); putAll(env);
        }
    }

    public void assign(String encoding) {
        clear();

        PrimitiveIterator.OfInt in = encoding.chars().iterator();
        while (in.hasNext()) {
            String var   = deQuote(encoding, in, '=');
            String value = deQuote(encoding, in, ';');
            this.put(var, value);
        }
    }

    protected void enQuote(StringBuilder sb, String value) {
        for(int n = 0, m = value.length() ; n < m ; ++n) {
            char c = value.charAt(n);
            if (c == '\\' || c == ';')
                sb.append('\\');
            sb.append(c);
        }
    }

    protected String deQuote(String encoding, PrimitiveIterator.OfInt in, char end) {
        String value = "";
        char c = '\0';
        while (in.hasNext()) {
            c = (char) in.nextInt();
            if (c == ';' || c == end) {
                break;
            } else if (c != '\\') {
                value += c;
            } else {
                if (!in.hasNext())
                    throw new RuntimeException(
                        "Escape char '\\' must not be the last" +
                        " char in \""+encoding+"\"!");
                value += (char) in.nextInt();
            }
        }
        if (c != end && (end != ';' || in.hasNext()))
            throw new RuntimeException(
                    "Expected '"+end+"' after \""+value+"\"!");
        return value;
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
