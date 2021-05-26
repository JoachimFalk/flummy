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
import java.util.Collection;

import org.opt4j.core.Individual;
import org.opt4j.core.Objective;
import org.opt4j.core.Objectives;
import org.opt4j.core.Value;
import org.opt4j.core.common.logger.Logger;
import org.opt4j.core.common.logger.TsvLogger;
import org.opt4j.core.optimizer.Archive;
import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.VPC.helper.TempDirectoryHandler;

import net.sf.opendse.model.Specification;
import net.sf.opendse.optimization.ImplementationWrapper;

public class VPCLogger extends TsvLogger implements Logger {

    @Inject
    public VPCLogger(Archive archive,
            @Constant(namespace = TsvLogger.class, value = "filename")
            String filename
          , @Constant(namespace = TsvLogger.class, value = "evaluationStep")
            int evaluationStep
          , @Constant(namespace = TsvLogger.class, value = "iterationStep")
            int iterationStep
          , @Constant(namespace = VPCLogger.class, value = "logWorkingFolder")
            boolean logWorkingFolder)
    {
        super(archive, filename, evaluationStep, iterationStep);
        this.logWorkingFolder = logWorkingFolder;
        this.out = initWriter(filename);
    }

    private final PrintWriter out;
    private long startTime = -1;

    private final boolean logWorkingFolder;

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
                if (!feasible) {
                    System.out.println("Is infeasible");
                    continue;
                }
            }
            if (logWorkingFolder) {
                try {
                    TempDirectoryHandler tempDirectoryHandler = TempDirectoryHandler.getTempDirectoryHandler(implementation);
                    File tmpFolder = tempDirectoryHandler.getDirectory();
                    String tmppath = getColumnDelimiter() + tmpFolder.getCanonicalPath();
                    objectivesText += tmppath;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
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
        out.println(header);
    }

}
