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

import org.opt4j.core.common.logger.LoggerModule;
import org.opt4j.core.common.logger.TsvLogger;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Panel;
import org.opt4j.core.start.Constant;

import de.fau.scd.VPC.config.properties.AttributeLogs;
import de.fau.scd.VPC.config.visualization.PropertyPanel;

@Panel(value = PropertyPanel.class)
public class VPCLoggerModule extends LoggerModule {

    @Info("Log working folder, e.g., where VPC simulations are performed.")
    @Order(10)
    @Constant(namespace = VPCLogger.class, value = "logWorkingFolder")
    protected boolean logWorkingFolder = false;

    public boolean getLogWorkingFolder() {
        return logWorkingFolder;
    }

    public void setLogWorkingFolder(boolean logWorkingFolder) {
        this.logWorkingFolder = logWorkingFolder;
    }

    @SuppressWarnings("serial")
    protected static class AttributeLogsImpl
    extends
        AttributeLogs
    implements
        VPCLogger.AttributeLogs
    {
    }
    @Info("Additional attributes to log")
//  @Order(3)
//  @Required(property = "dfgSource", elements = { "DFG_FROM_SIM_EXPORT" })
    protected final AttributeLogsImpl attributeLogs = new AttributeLogsImpl();

    public AttributeLogs getAttributeLogs() {
        return attributeLogs;
    }

    public void setAttributeLogs(AttributeLogs als) {
        this.attributeLogs.assign(als);
    }

    @Override
    public void config() {
        bind(VPCLogger.class).in(SINGLETON);
        addIndividualStateListener(VPCLogger.class);
        addOptimizerIterationListener(VPCLogger.class);
        addOptimizerStateListener(VPCLogger.class);

        int evaluationStep = this.evaluationStep;
        int iterationStep = this.iterationStep;

        if (!loggingPerEvaluation) {
            evaluationStep = -1;
        }
        if (!loggingPerIteration) {
            iterationStep = -1;
        }
        bindConstant("evaluationStep", TsvLogger.class)
            .to(evaluationStep);
        bindConstant("iterationStep", TsvLogger.class)
            .to(iterationStep);

        bind(VPCLogger.AttributeLogs.class)
            .toInstance(attributeLogs);
    }
}
