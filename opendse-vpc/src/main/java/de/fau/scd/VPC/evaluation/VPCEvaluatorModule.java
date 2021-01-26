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

import org.opt4j.core.config.annotations.File;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Required;
import org.opt4j.core.config.annotations.Panel;

import net.sf.opendse.optimization.evaluator.EvaluatorModule;

import de.fau.scd.VPC.config.annotations.Text;

import de.fau.scd.VPC.config.properties.Arguments;
import de.fau.scd.VPC.config.properties.Environment;
import de.fau.scd.VPC.config.properties.Objectives;

import de.fau.scd.VPC.config.visualization.PropertyPanel;

import de.fau.scd.VPC.evaluation.VPCEvaluator.FireActorInLoop;
import de.fau.scd.VPC.evaluation.VPCEvaluator.SchedulerType;
import de.fau.scd.VPC.evaluation.VPCEvaluator.TimeSlice;
import de.fau.scd.VPC.evaluation.VPCEvaluator.TraceType;
import de.fau.scd.VPC.evaluation.VPCEvaluator.VPCConfigTemplate;
import de.fau.scd.VPC.evaluation.VPCEvaluator.SimulatorExecutable;

@Panel(value = PropertyPanel.class)
public class VPCEvaluatorModule extends EvaluatorModule {

    @Info("The VPC simulator executable")
    @Order(1)
    @File
    protected String simulatorExecutable = "";

    public String getSimulatorExecutable() {
        return simulatorExecutable;
    }

    public void setSimulatorExecutable(String exe) {
        this.simulatorExecutable = exe;
    }


    protected static class SimulatorArgumentsImpl
    extends
        Arguments
    implements
        VPCEvaluator.SimulatorArguments
    {
    }

    @Info("Arguments for the VPC simulator executable")
    @Order(2)
    @Text
    protected SimulatorArgumentsImpl simulatorArguments = new SimulatorArgumentsImpl();

    public String getSimulatorArguments() {
        return simulatorArguments.toString();
    }

    public void setSimulatorArguments(String simulatorArguments) {
        this.simulatorArguments.assign(simulatorArguments);
    }

    @SuppressWarnings("serial")
    protected static class SimulatorEnvironmentImpl
        extends
            Environment
        implements
            VPCEvaluator.SimulatorEnvironment
    {
    }

    @Info("Environment for the VPC simulator executable")
    @Order(3)
    protected final SimulatorEnvironmentImpl simulatorEnvironment = new SimulatorEnvironmentImpl();

    public Environment getSimulatorEnvironment() {
        return simulatorEnvironment;
    }

    public void setSimulatorEnvironment(Environment env) {
        this.simulatorEnvironment.assign(env);
    }

    @SuppressWarnings("serial")
    protected static class VPCObjectivesImpl
        extends
            Objectives
        implements
            VPCEvaluator.VPCObjectives
    {
    }

    @Info("The VPC configuration template.")
    @Order(10)
    @File
    protected String vpcConfigTemplate = "";

    public String getVpcConfigTemplate() {
        return vpcConfigTemplate;
    }

    public void setVpcConfigTemplate(String vpcConfigTemplate) {
        this.vpcConfigTemplate = vpcConfigTemplate;
    }

    @Info("Objectives of the VPC evaluator")
    @Order(15)
    protected final VPCObjectivesImpl objectives = new VPCObjectivesImpl();

    public Objectives getObjectives() {
        return objectives;
    }

    public void setObjectives(Objectives objs) {
        if (objs != objectives) {
            this.objectives.clear();
            this.objectives.addAll(objs);
        }
    }

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

    @Info("Choose the scheduler for the VPC simulation")
    @Order(20)
    protected SchedulerTypeEnum schedulerType = SchedulerTypeEnum.RRNOPRE;

    public SchedulerTypeEnum getSchedulerType() {
        return schedulerType;
    }

    public void setSchedulerType(SchedulerTypeEnum schedulerType) {
        this.schedulerType = schedulerType;
    }

    @Order(21)
    @Required(property = "schedulerType", elements = { "RR" })
    protected double timeSlice = 0.00017;

    public double getTimeSlice() {
        return this.timeSlice;
    }

    public void setTimeSlice(double timeSlice) {
        this.timeSlice = timeSlice;
    }

    @Order(22)
    @Required(property = "schedulerType", elements = { "RRNOPRE" })
    protected boolean fireActorInLoop = false;

    public boolean getFireActorInLoop() {
        return this.fireActorInLoop;
    }

    public void setFireActorInLoop(boolean fireActorInLoop) {
        this.fireActorInLoop = fireActorInLoop;
    }

    public enum TraceTypeEnum {
        PAJE,
        VCD
    }

    @Order(30)
    @Info("Select the trace format")
    protected TraceTypeEnum traceType = TraceTypeEnum.PAJE;

    public TraceTypeEnum getTraceType() {
        return this.traceType;
    }

    public void setTraceType(TraceTypeEnum traceType) {
        this.traceType = traceType;
    }

    @Override
    protected void config() {
        bindConstant(SimulatorExecutable.class).to(simulatorExecutable);
        bind(VPCEvaluator.SimulatorEnvironment.class).toInstance(simulatorEnvironment);
        bind(VPCEvaluator.SimulatorArguments.class).toInstance(simulatorArguments);
        bind(VPCEvaluator.VPCObjectives.class).toInstance(objectives);
        bindConstant(SchedulerType.class).to(schedulerType);
        bindConstant(TimeSlice.class).to(timeSlice);
        bindConstant(FireActorInLoop.class).to(fireActorInLoop);
        bindConstant(TraceType.class).to(traceType);
        bindConstant(VPCConfigTemplate.class).to(vpcConfigTemplate);
        bindEvaluator(VPCEvaluator.class);
    }

}
