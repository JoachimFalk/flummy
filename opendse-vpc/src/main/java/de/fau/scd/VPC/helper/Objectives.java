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

import java.util.Collections;
import java.util.Iterator;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.PrimitiveIterator;
import java.util.TreeMap;
import java.util.Map.Entry;
import java.util.function.IntConsumer;

@SuppressWarnings("serial")
public class Objectives extends TreeMap<String, ObjectiveInfo> {

    public Objectives() {
        super();
    }

    public Objectives(String encoding) {
        super();

        CharIterator in = new CharIterator(encoding);

        while (in.hasNext()) {
            String objName    = deQuote(in, ',');
            String objSign    = deQuote(in, ',');
            String parseFile  = deQuote(in, ',');
            String parseRegex = deQuote(in, ';');
            this.put(objName, new ObjectiveInfo(objSign, parseFile, parseRegex));
        }
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();

        Iterator<Entry<String, ObjectiveInfo>> i = entrySet().iterator();
        while (i.hasNext()) {
            Entry<String, ObjectiveInfo> e = i.next();
            enQuote(sb, e.getKey());
            sb.append(',');
            enQuote(sb, e.getValue().getObjSign().name());
            sb.append(',');
            enQuote(sb, e.getValue().getParseFile().toString());
            sb.append(',');
            enQuote(sb, e.getValue().getParseRegex().pattern());
            if (i.hasNext())
                sb.append(';');
        }
        return sb.toString();
    }

    protected void enQuote(StringBuilder sb, String value) {
        for(int n = 0, m = value.length() ; n < m ; ++n) {
            char c = value.charAt(n);
            if (c == '\\' || c == ';' || c == ',')
                sb.append('\\');
            sb.append(c);
        }
    }

    static class CharIterator implements PrimitiveIterator.OfInt {
        int cur = 0;

        CharIterator(String str) {
            this.str = str;
        }

        public boolean hasNext() {
            return cur < str.length();
        }

        public int nextInt() {
            if (hasNext()) {
                return str.charAt(cur++);
            } else {
                throw new NoSuchElementException();
            }
        }

        @Override
        public void forEachRemaining(IntConsumer block) {
            for (; cur < str.length(); cur++) {
                block.accept(str.charAt(cur));
            }
        }

        public String getString() {
            return str;
        }

        protected final String str;
    }

    protected String deQuote(CharIterator in, char end) {
        String value = "";
        char c = '\0';
        while (in.hasNext()) {
            c = (char) in.nextInt();
            if (c == ';' || c == ',' || c == end) {
                break;
            } else if (c != '\\') {
                value += c;
            } else {
                if (!in.hasNext())
                    throw new RuntimeException(
                        "Escape char '\\' must not be the last" +
                        " char in \""+in.getString()+"\"!");
                value += (char) in.nextInt();
            }
        }
        if (c != end && (end != ';' || in.hasNext()))
            throw new RuntimeException(
                    "Expected '"+end+"' after \""+value+"\"!");
        return value;
    }

    public Map<String, ObjectiveInfo> getObjectives() {
        return Collections.unmodifiableMap(this);
    }
}
