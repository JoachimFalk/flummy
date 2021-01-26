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

import java.util.ArrayList;
import java.util.PrimitiveIterator;

public class Arguments {
    
    private ArrayList<String> argsSplit = new ArrayList<String>();
    private String            argsString = "";
    
    public Arguments() {
    }

    public Arguments(String args) {
        assign(args);
    }
    
    public void assign(Arguments args) {
        argsString = args.argsString;
        argsSplit  = args.argsSplit;
    }
    
    public void assign(String input) {
        argsSplit  = new ArrayList<String>();
        
        PrimitiveIterator.OfInt in = input.chars().iterator();
        
        StringBuilder args = new StringBuilder();
        StringBuilder arg  = null;
        
        try {
            while (in.hasNext()) {
                char c = (char) in.nextInt();
                if (Character.isWhitespace(c)) {
                    args.append(c);
                    if (arg != null) {
                        argsSplit.add(arg.toString());
                        arg = null;                        
                    }
                } else {
                    if (arg == null)
                        arg = new StringBuilder();
                    if (c == '"') {
                        args.append(c);
                        while (true) {
                            if (!in.hasNext()) {
                                args.append('"');
                                throw new RuntimeException(
                                        "Unclosed double quoted string in: "+input);
                            }
                            c = (char) in.nextInt();
                            if (c == '"') {
                                args.append(c);
                                break;
                            } else if (c == '\\') {
                                if (!in.hasNext())
                                    throw new RuntimeException(
                                            "Escape char '\\' must not be the last char in: "+input);
                                args.append(c);
                                c = (char) in.nextInt();
                                arg.append(c);
                            } else {
                                args.append(c);
                                arg.append(c);                                
                            }
                        }
                    } else if (c == '\'') {
                        args.append(c);
                        while (true) {
                            if (!in.hasNext()) {
                                args.append('\'');
                                throw new RuntimeException(
                                        "Unclosed singled quoted string in: "+input);
                            }
                            c = (char) in.nextInt();                    
                            if (c == '\'') {
                                args.append(c);
                                break;
                            } else {
                                args.append(c);
                                arg.append(c);                                
                            }
                        }
                    } else if (c == '\\') {
                        if (!in.hasNext())
                            throw new RuntimeException(
                                    "Escape char '\\' must not be the last char in: "+input);
                        args.append(c);
                        c = (char) in.nextInt();
                        args.append(c);
                        arg.append(c);
                    } else {
                        args.append(c);
                        arg.append(c);                                
                    }
                }
            }
        } finally {
            if (arg != null) {
                argsSplit.add(arg.toString());
                arg = null;                        
            }                    
            argsString = args.toString();
        }
    }
    
    public String toString() {
        return argsString;
    }
    
    public String [] getArguments() {
        return argsSplit.toArray(new String[0]);
    }
}
