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
package de.fau.scd.VPC.output;

import static org.opt4j.core.Objective.INFEASIBLE;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;

import java.util.*;
import java.util.regex.Matcher;

import org.opt4j.core.Individual;
import org.opt4j.core.Objective;
import org.opt4j.core.Objectives;
import org.opt4j.core.Value;
import org.opt4j.core.common.logger.Logger;
import org.opt4j.core.common.logger.TsvLogger;
import org.opt4j.core.optimizer.Archive;
import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.VPC.config.properties.AttributeLog;
import de.fau.scd.VPC.helper.TempDirectoryHandler;
import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.ImplementationWrapper;
import net.sf.opendse.optimization.SpecificationWrapper;

public class VPCLogger extends TsvLogger implements Logger {

    interface AttributeLogs {
        public List<AttributeLog> getAttributeLogs();
    }

    @Inject
    public VPCLogger(
        Archive archive
      , SpecificationWrapper specWrapper
      , @Constant(namespace = TsvLogger.class, value = "filename")
        String filename
      , @Constant(namespace = TsvLogger.class, value = "evaluationStep")
        int evaluationStep
      , @Constant(namespace = TsvLogger.class, value = "iterationStep")
        int iterationStep
      , @Constant(namespace = VPCLogger.class, value = "logWorkingFolder")
        boolean logWorkingFolder
      , @Constant(namespace = VPCLogger.class, value = "logInfeasible")
        boolean logInfeasible
      , AttributeLogs attributeLogs)
    {
        super(archive, filename, evaluationStep, iterationStep);

        Specification specification = specWrapper.getSpecification();

        ArrayList<String> taskIds = new ArrayList<>();
        for (Task t : specification.getApplication())
            taskIds.add(t.getId());
        taskIds.sort(null);
        ArrayList<String> resIds = new ArrayList<>();
        for (Resource r : specification.getArchitecture())
            resIds.add(r.getId());
        resIds.sort(null);
        ArrayList<String> mapIds = new ArrayList<>();
        for (Mapping<Task, Resource> m : specification.getMappings())
            mapIds.add(m.getId());
        mapIds.sort(null);

        for (AttributeLog al : attributeLogs.getAttributeLogs()) {
            switch (al.getNodeType()) {
                case APPLICATION:
                    addAttrLogLookup(taskIds, applicationAttrLogEntries, al);
                    break;
                case ARCHITECTURE:
                    addAttrLogLookup(resIds, architectureAttrLogEntries, al);
                    break;
                case MAPPINGS:
                    addAttrLogLookup(mapIds, mappingsAttrLogEntries, al);
                    break;
                case IMPLEMENTATION:
                    if (al.getElemRegex().pattern().isEmpty()) {
                        AttrLogEntry ale = new AttrLogEntry(al.getAttrName());
                        attrLogEntries.add(ale);
                        implementationAttrLogEntries.add(ale);
                    } else {
                        addAttrLogLookup(taskIds, applicationAttrLogEntries, al);
                        addAttrLogLookup(resIds, architectureAttrLogEntries, al);
                        addAttrLogLookup(mapIds, mappingsAttrLogEntries, al);
                    }
                    break;
            }
        }

        this.logWorkingFolder = logWorkingFolder;
        this.logInfeasibles = logInfeasible;
        this.out = initWriter(filename);
    }

    private void addAttrLogLookup(
        ArrayList<String> ids
      , Map<String, List<AttrLogEntry> > lookup
      , AttributeLog al)
    {
        for (String id : ids) {
            Matcher m = al.getElemRegex().matcher(id);
            if (m.find()) {
                AttrLogEntry ale = new AttrLogEntry(id, al.getAttrName());
                attrLogEntries.add(ale);
                List<AttrLogEntry> perIdAttrLogEntries = lookup.get(id);
                if (perIdAttrLogEntries == null) {
                    perIdAttrLogEntries = new ArrayList<>();
                    lookup.put(id, perIdAttrLogEntries);
                }
                perIdAttrLogEntries.add(ale);
            }
        }
    }

    private static class AttrLogEntry {

        private final String id;
        private final String attrName;

        private Object value = null;

        public AttrLogEntry(String id, String attrName) {
            this.id       = null;
            this.attrName = attrName;
        }
        public AttrLogEntry(String attrName) {
            this.id       = null;
            this.attrName = attrName;
        }

        public String getName() {
            if (id != null)
                return id + "/" + attrName;
            else
                return attrName;
        }

        public String getAttrName() {
            return attrName;
        }

        public Object getValue() {
            return value;
        }

        public void setValue(Object value) {
            assert this.value == null;
            this.value = value;
        }

        public void reset() {
            this.value = null;
        }

    }


    private final PrintWriter out;
    private long startTime = -1;

    private final boolean logWorkingFolder;
    private final boolean logInfeasibles;

    private final List<AttrLogEntry> attrLogEntries = new ArrayList<>();
    private final Map<String, List<AttrLogEntry> > applicationAttrLogEntries = new HashMap<>();
    private final Map<String, List<AttrLogEntry> > architectureAttrLogEntries = new HashMap<>();
    private final Map<String, List<AttrLogEntry> > mappingsAttrLogEntries = new HashMap<>();
    private final List<AttrLogEntry> implementationAttrLogEntries = new ArrayList<>();

    /*
     * (non-Javadoc)
     *
     * @see org.opt4j.common.logger.AbstractLogger#logEvent(int, int)
     */
    @Override
    public void logEvent(int iteration, int evaluation) {
        assert startTime != -1 : "not initialized";
        double time = ((double) System.currentTimeMillis() - startTime) / 1000.0;

        for (Individual individual : archive) {
            ImplementationWrapper pheno = (ImplementationWrapper) individual.getPhenotype();
            Specification implementation = pheno.getImplementation();
            Objectives objectives = individual.getObjectives();

            String statisticsText = getStatistics(iteration, evaluation, time);
            String objectivesText = getIndividual(individual);

            {
                Boolean feasible = true;
                for (Value<?> value : objectives.getValues()) {
                    if (value == INFEASIBLE || value == null || value.getValue() == null) {
                        feasible = false;
                        break;
                    }
                }
                if (!feasible && !logInfeasibles)
                    continue;
            }
            if (logWorkingFolder) {
                try {
                    TempDirectoryHandler tempDirectoryHandler = TempDirectoryHandler.getTempDirectoryHandler(implementation);
                    File tmpFolder = tempDirectoryHandler.getDirectory();
                    objectivesText += getColumnDelimiter() + tmpFolder.getCanonicalPath();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            for (AttrLogEntry ale : attrLogEntries)
                ale.reset();
            if (!applicationAttrLogEntries.isEmpty())
                for (Task t : implementation.getApplication()) {
                    List<AttrLogEntry> ales = applicationAttrLogEntries.get(t.getId());
                    if (ales == null)
                        continue;
                    for (AttrLogEntry ale : ales) {
                        ale.setValue(t.getAttribute(ale.getAttrName()));
                    }
                }
            if (!architectureAttrLogEntries.isEmpty())
                for (Resource r : implementation.getArchitecture()) {
                    List<AttrLogEntry> ales = architectureAttrLogEntries.get(r.getId());
                    if (ales == null)
                        continue;
                    for (AttrLogEntry ale : ales) {
                        ale.setValue(r.getAttribute(ale.getAttrName()));
                    }
                }
            if (!mappingsAttrLogEntries.isEmpty())
                for (Mapping<Task, Resource> m : implementation.getMappings()) {
                    List<AttrLogEntry> ales = mappingsAttrLogEntries.get(m.getId());
                    if (ales == null)
                        continue;
                    for (AttrLogEntry ale : ales) {
                        ale.setValue(m.getAttribute(ale.getAttrName()));
                    }
                }
            for (AttrLogEntry ale : implementationAttrLogEntries) {
                ale.setValue(implementation.getAttribute(ale.getAttrName()));
            }
            for (AttrLogEntry ale : attrLogEntries)
                objectivesText += getColumnDelimiter() + ale.getValue();

            out.println(statisticsText + objectivesText);
        }
        out.flush();
    }

    @Override
    public void optimizationStarted() {
        startTime = System.currentTimeMillis();
    }

    /*
     * (non-Javadoc)
     *
     * @see org.opt4j.common.logger.AbstractLogger#optimizationStopped()
     */
    @Override
    public void optimizationStopped() {
        out.close();
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * org.opt4j.common.logger.AbstractLogger#logHeader(java.util.Collection)
     */
    @Override
    public void logHeader(Collection<Objective> objectives) {
        String header = getCommentDelimiter() + "iteration" + getColumnDelimiter() + "evaluations"
                + getColumnDelimiter() + "runtime[s]";
        for (Objective objective : objectives) {
            String name = objective.getName().replaceAll("[ \n\t\r]", "_");
            header += getColumnDelimiter() + name + "[" + objective.getSign() + "]";
        }
        if (logWorkingFolder)
            header += getColumnDelimiter() + "workingFolder";
        for (AttrLogEntry e : attrLogEntries) {
            header += getColumnDelimiter() + e.getName();
        }
        out.println(header);
    }

}
