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

import java.io.File;
import java.util.regex.Pattern;
import org.opt4j.core.Objective.Sign;
import org.opt4j.core.Objective;

public class ObjectiveInfo {

    public ObjectiveInfo(
        String  objName
      , Sign    objSign
      , File    parseFile
      , Pattern parseRegex)
    {
        this.objective  = new Objective(objName
                , objSign);
        this.parseFile  = parseFile;
        this.parseRegex = parseRegex;
    }

    public ObjectiveInfo(
            String  objName
          , String  objSign
          , String  parseFile
          , String  parseRegex)
    {
        this.objective  = new Objective(objName
                , Sign.valueOf(objSign));
        this.parseFile  = new File(parseFile);
        this.parseRegex = Pattern.compile(parseRegex);
    }

    public Objective getObjective() {
        return objective;
    }

    public String getObjName() {
        return objective.getName();
    }

    public Sign getObjSign() {
        return objective.getSign();
    }

    public File getParseFile() {
        return parseFile;
    }

    public Pattern getParseRegex() {
        return parseRegex;
    }

    protected final Objective objective;
    protected final File      parseFile;
    protected final Pattern   parseRegex;
}
