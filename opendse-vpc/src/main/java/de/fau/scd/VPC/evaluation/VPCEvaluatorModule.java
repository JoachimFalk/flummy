// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
 *   2020 FAU -- Martin Letras <martin.letras@fau.de>
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

package de.fau.scd.VPC.evaluation;

//import static java.lang.annotation.RetentionPolicy.RUNTIME;

//import java.lang.annotation.Retention;

import org.opt4j.core.config.annotations.File;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Required;

import de.fau.scd.VPC.evaluation.VPCEvaluator.FireActorInLoop;
import de.fau.scd.VPC.evaluation.VPCEvaluator.NumberOfIterations;
import de.fau.scd.VPC.evaluation.VPCEvaluator.SchedulerType;
import de.fau.scd.VPC.evaluation.VPCEvaluator.ExecutableOfSimulation;
import de.fau.scd.VPC.evaluation.VPCEvaluator.TimeSlice;
import de.fau.scd.VPC.evaluation.VPCEvaluator.TraceType;
import de.fau.scd.VPC.evaluation.VPCEvaluator.VPCConfigTemplate;
import net.sf.opendse.optimization.evaluator.EvaluatorModule;

public class VPCEvaluatorModule extends EvaluatorModule {
    public enum SchedulerTypeEnum {
        /**
         * Use TDMA scheduler
         */
        TDMA,
        /**
         * Use FLEXRAY scheduler
         */
        FLEXRAY,
        /**
         * Use TTCC scheduler
         */
        TTCC,
        /**
         * Use Round Robin scheduler
         */
        RR,
        /**
         * Use Round Robin no-preemption scheduler
         */
        RRNOPRE,
        /**
         * Use Static Priority scheduler
         */
        SP,
        /**
         * Use Static Priority no-preemption scheduler
         */
        SPNOPRE,
        /**
         * Use First come first served scheduler
         */
        FCFS,
        /**
         * Use Rate monotonic scheduler
         */
        RM,
        /**
         * Use AVB scheduler
         */
        AVB,
        /**
         * Use MOST scheduler
         */
        MOST,
        /**
         * Use STREAMSHAPER scheduler
         */
        STREAMSHAPER
    }

    public enum TraceTypeEnum{
        PAJE,
        VCD
    }

    @Info("The VPC configuration template.")
    @Order(1)
    @File
    protected String vpcConfigTemplate = "";

    public String getVpcConfigTemplate() {
        return vpcConfigTemplate;
    }

    public void setVpcConfigTemplate(String vpcConfigTemplate) {
        this.vpcConfigTemplate = vpcConfigTemplate;
    }

    @Info("The VPC simulation start script.")
    @Order(2)
    @File
    protected String executableOfSimulation = "";

    @Order(0)
    @Info("Set the number of iterations")
    protected int numberOfIterations = 100;

    @Order(0)
    @Info("Select the trace format")
    protected TraceTypeEnum traceType = TraceTypeEnum.PAJE;

    @Order(0)
    @Info("Choose the scheduler for the VPC simulation")
    protected SchedulerTypeEnum schedulerType = SchedulerTypeEnum.RRNOPRE;

    @Required(property = "schedulerType", elements = { "RR" })
    protected double timeSlice = 0.00017;

    @Required(property = "schedulerType", elements = { "RRNOPRE" })
    protected boolean fireActorInLoop = false;

    public TraceTypeEnum getTraceType() {
        return this.traceType;
    }

    public void setTraceType(TraceTypeEnum traceType) {
        this.traceType = traceType;
    }

    public boolean getFireActorInLoop() {
        return this.fireActorInLoop;
    }

    public void setFireActorInLoop(boolean fireActorInLoop) {
        this.fireActorInLoop = fireActorInLoop;
    }

    public double getTimeSlice() {
        return this.timeSlice;
    }

    public void setTimeSlice(double timeSlice) {
        this.timeSlice = timeSlice;
    }

    public int getNumberOfIterations() {
        return numberOfIterations;
    }

    public void setNumberOfIterations(int numberOfIterations) {
        this.numberOfIterations = numberOfIterations;
    }

    public SchedulerTypeEnum getSchedulerType() {
        return schedulerType;
    }

    public void setSchedulerType(SchedulerTypeEnum schedulerType) {
        this.schedulerType = schedulerType;
    }

    public String getExecutableOfSimulation() {
        return executableOfSimulation;
    }

    public void setExecutableOfSimulation(String executableOfSimulation) {
        this.executableOfSimulation = executableOfSimulation;
    }

    @Override
    protected void config() {
        bindConstant(ExecutableOfSimulation.class).to(executableOfSimulation);
        bindConstant(NumberOfIterations.class).to(numberOfIterations);
        bindConstant(SchedulerType.class).to(schedulerType);
        bindConstant(TimeSlice.class).to(timeSlice);
        bindConstant(FireActorInLoop.class).to(fireActorInLoop);
        bindConstant(TraceType.class).to(traceType);
        bindConstant(VPCConfigTemplate.class).to(vpcConfigTemplate);
        bindEvaluator(VPCEvaluator.class);
    }

}
