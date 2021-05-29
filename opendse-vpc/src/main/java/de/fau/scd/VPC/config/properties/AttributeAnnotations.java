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

package de.fau.scd.VPC.config.properties;

import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.LinkedList;
import java.util.PrimitiveIterator;

@SuppressWarnings("serial")
public class AttributeAnnotations extends LinkedList<AttributeAnnotation> {

    public AttributeAnnotations() {
        super();
    }

    public AttributeAnnotations(String encoding) {
        super();
        assign(encoding);
    }

    public void assign(AttributeAnnotations objs) {
        if (objs != this) {
            this.clear();
            this.addAll(objs);
        }
    }

    public void assign(String encoding) {
        clear();
        PrimitiveIterator.OfInt in = encoding.chars().iterator();

        while (in.hasNext()) {
            String argString = deQuote(encoding, in, ',');
            // Backward compatibility cruft
            AttributeAnnotation.NodeType nodeType =
                AttributeAnnotation.NodeType.SPECIFICATION;
            try {
                nodeType = AttributeAnnotation.NodeType.valueOf(argString);
                argString = deQuote(encoding, in, ',');
            } catch (IllegalArgumentException e) {}
            String elemRegex  = argString;
            String attrName   = deQuote(encoding, in, ',');
            String attrType   = deQuote(encoding, in, ',');
            String attrValue  = deQuote(encoding, in, ';');
            this.add(new AttributeAnnotation(
                nodeType
              , elemRegex
              , attrName
              , AttributeAnnotation.AttrType.valueOf(attrType)
              , attrValue));
        }
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();

        Iterator<AttributeAnnotation> i = iterator();
        while (i.hasNext()) {
            AttributeAnnotation aa = i.next();
            enQuote(sb, aa.getNodeType().name());
            sb.append(',');
            enQuote(sb, aa.getElemRegex().pattern());
            sb.append(',');
            enQuote(sb, aa.getAttrName());
            sb.append(',');
            enQuote(sb, aa.getAttrType().name());
            sb.append(',');
            enQuote(sb, aa.getAttrValue().toString());
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

    protected String deQuote(String encoding, PrimitiveIterator.OfInt in, char end) {
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
                        " char in \""+encoding+"\"!");
                value += (char) in.nextInt();
            }
        }
        if (c != end && (end != ';' || in.hasNext()))
            throw new RuntimeException(
                    "Expected '"+end+"' after \""+value+"\"!");
        return value;
    }

    public List<AttributeAnnotation> getAttributeAnnotation() {
        return Collections.unmodifiableList(this);
    }
}
