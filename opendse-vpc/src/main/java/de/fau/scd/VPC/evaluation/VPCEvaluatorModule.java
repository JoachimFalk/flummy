// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
 *   2020 FAU -- Martin Letras <martin.letras@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

package de.fau.scd.VPC.evaluation;

//import static java.lang.annotation.RetentionPolicy.RUNTIME;
//import java.lang.annotation.Retention;
//import com.google.inject.BindingAnnotation;

import org.opt4j.core.config.annotations.File;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Panel;
import org.opt4j.core.config.annotations.Required;
import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import net.sf.opendse.optimization.evaluator.EvaluatorModule;

import de.fau.scd.VPC.config.annotations.Text;

import de.fau.scd.VPC.config.properties.Arguments;
import de.fau.scd.VPC.config.properties.Environment;
import de.fau.scd.VPC.config.properties.Objectives;

import de.fau.scd.VPC.config.visualization.PropertyPanel;
import de.fau.scd.VPC.io.SpecificationWrapperSNG;

@Panel(value = PropertyPanel.class)
public class VPCEvaluatorModule extends EvaluatorModule {

    @Info("Reuse setup from SNGReader. To enable this, you must have an SNGReader module configured with DFG_FROM_SIM_EXPORT.")
    @Order(0)
    protected boolean useSNGReaderSetup = false;

    public boolean getUseSNGReaderSetup() {
        return useSNGReaderSetup;
    }

    public void setUseSNGReaderSetup(boolean useSNGReaderSetup) {
        this.useSNGReaderSetup = useSNGReaderSetup;
    }

    @Info("The VPC simulator executable")
    @Order(1)
    @Constant(namespace = VPCEvaluatorModule.class, value = "simulatorExecutable")
    @Required(property = "useSNGReaderSetup", value = false)
    @File
    protected String simulatorExecutable = "";

    public String getSimulatorExecutable() {
        return simulatorExecutable;
    }

    public void setSimulatorExecutable(String exe) {
        this.simulatorExecutable = exe;
    }

    protected static class SimulatorArgumentsImpl extends Arguments {
    }

    @Info("Arguments for the VPC simulator executable")
    @Order(2)
    @Required(property = "useSNGReaderSetup", value = false)
    @Text
    protected SimulatorArgumentsImpl simulatorArguments = new SimulatorArgumentsImpl();

    public String getSimulatorArguments() {
        return simulatorArguments.toString();
    }

    public void setSimulatorArguments(String simulatorArguments) {
        this.simulatorArguments.assign(simulatorArguments);
    }

    @SuppressWarnings("serial")
    protected static class SimulatorEnvironmentImpl extends Environment {
    }

    @Info("Environment for the VPC simulator executable")
    @Order(3)
    @Required(property = "useSNGReaderSetup", value = false)
    protected final SimulatorEnvironmentImpl simulatorEnvironment = new SimulatorEnvironmentImpl();

    public Environment getSimulatorEnvironment() {
        return simulatorEnvironment;
    }

    public void setSimulatorEnvironment(Environment env) {
        this.simulatorEnvironment.assign(env);
    }

    @Info("The VPC configuration template.")
    @Order(10)
    @Constant(namespace = VPCEvaluatorModule.class, value = "vpcConfigTemplate")
    @Required(property = "useSNGReaderSetup", value = false)
    @File
    protected String vpcConfigTemplate = "";

    public String getVpcConfigTemplate() {
        return vpcConfigTemplate;
    }

    public void setVpcConfigTemplate(String vpcConfigTemplate) {
        this.vpcConfigTemplate = vpcConfigTemplate;
    }

    protected static class SimInfoLocal
    implements
        VPCEvaluator.SimInfo
    {
        @Inject
        public SimInfoLocal(
            @Constant(namespace = VPCEvaluatorModule.class, value = "simulatorExecutable")
            String                   simulatorExecutable
          , SimulatorArgumentsImpl   simulatorArguments
          , SimulatorEnvironmentImpl simulatorEnvironment
          , @Constant(namespace = VPCEvaluatorModule.class, value = "vpcConfigTemplate")
            String                   vpcConfigTemplate
            )
        {
            this.simExecutable     = simulatorExecutable;
            this.simArguments      = simulatorArguments.getArguments();
            this.simEnvironment    = simulatorEnvironment.getEnvironment();
            this.vpcConfigTemplate = vpcConfigTemplate;
        }

        @Override
        public String getExecutable() {
            return simExecutable;
        }

        @Override
        public String[] getArguments() {
            return simArguments;
        }

        @Override
        public String[] getEnvironment() {
            return simEnvironment;
        }

        @Override
        public String getVpcConfigTemplate() {
            return vpcConfigTemplate;
        }

        protected final String   simExecutable;
        protected final String[] simArguments;
        protected final String[] simEnvironment;
        protected final String   vpcConfigTemplate;
    }

    protected static class SimInfoSNGReader
    implements
        VPCEvaluator.SimInfo
    {
        @Inject
        public SimInfoSNGReader(
            @Constant(namespace = SpecificationWrapperSNG.class, value = "simulatorExecutable")
            String               simulatorExecutable
          , SpecificationWrapperSNG.SimulatorArguments
                                 simulatorArguments
          , SpecificationWrapperSNG.SimulatorEnvironment
                                 simulatorEnvironment
          , @Constant(namespace = SpecificationWrapperSNG.class, value = "vpcConfigTemplate")
            String               vpcConfigTemplate
            )
        {
            this.simExecutable     = simulatorExecutable;
            this.simArguments      = simulatorArguments.getArguments();
            this.simEnvironment    = simulatorEnvironment.getEnvironment();
            this.vpcConfigTemplate = vpcConfigTemplate;
        }

        @Override
        public String getExecutable() {
            return simExecutable;
        }

        @Override
        public String[] getArguments() {
            return simArguments;
        }

        @Override
        public String[] getEnvironment() {
            return simEnvironment;
        }

        @Override
        public String getVpcConfigTemplate() {
            return vpcConfigTemplate;
        }

        protected final String   simExecutable;
        protected final String[] simArguments;
        protected final String[] simEnvironment;
        protected final String   vpcConfigTemplate;
    }


    @SuppressWarnings("serial")
    protected static class VPCObjectivesImpl
        extends
            Objectives
        implements
            VPCEvaluator.VPCObjectives
    {
    }

    @Info("Objectives of the VPC evaluator")
    @Order(15)
    protected final VPCObjectivesImpl objectives = new VPCObjectivesImpl();

    public Objectives getObjectives() {
        return objectives;
    }

    public void setObjectives(Objectives objs) {
        this.objectives.assign(objs);
    }

//  @Retention(RUNTIME)
//  @BindingAnnotation
//  public @interface SchedulerType {
//  }
//
//  @Retention(RUNTIME)
//  @BindingAnnotation
//  public @interface TimeSlice{
//  }
//
//  @Retention(RUNTIME)
//  @BindingAnnotation
//  public @interface FireActorInLoop{
//  }
//
//  @Retention(RUNTIME)
//  @BindingAnnotation
//  public @interface TraceType{
//  }
//
//  public enum SchedulerTypeEnum {
//      /**
//       * Use TDMA scheduler
//       */
//      TDMA,
//      /**
//       * Use FLEXRAY scheduler
//       */
//      FLEXRAY,
//      /**
//       * Use TTCC scheduler
//       */
//      TTCC,
//      /**
//       * Use Round Robin scheduler
//       */
//      RR,
//      /**
//       * Use Round Robin no-preemption scheduler
//       */
//      RRNOPRE,
//      /**
//       * Use Static Priority scheduler
//       */
//      SP,
//      /**
//       * Use Static Priority no-preemption scheduler
//       */
//      SPNOPRE,
//      /**
//       * Use First come first served scheduler
//       */
//      FCFS,
//      /**
//       * Use Rate monotonic scheduler
//       */
//      RM,
//      /**
//       * Use AVB scheduler
//       */
//      AVB,
//      /**
//       * Use MOST scheduler
//       */
//      MOST,
//      /**
//       * Use STREAMSHAPER scheduler
//       */
//      STREAMSHAPER
//  }
//
//  @Info("Choose the scheduler for the VPC simulation")
//  @Order(20)
//  protected SchedulerTypeEnum schedulerType = SchedulerTypeEnum.RRNOPRE;
//
//  public SchedulerTypeEnum getSchedulerType() {
//      return schedulerType;
//  }
//
//  public void setSchedulerType(SchedulerTypeEnum schedulerType) {
//      this.schedulerType = schedulerType;
//  }
//
//  @Order(21)
//  @Required(property = "schedulerType", elements = { "RR" })
//  protected double timeSlice = 0.00017;
//
//  public double getTimeSlice() {
//      return this.timeSlice;
//  }
//
//  public void setTimeSlice(double timeSlice) {
//      this.timeSlice = timeSlice;
//  }
//
//  @Order(22)
//  @Required(property = "schedulerType", elements = { "RRNOPRE" })
//  protected boolean fireActorInLoop = false;
//
//  public boolean getFireActorInLoop() {
//      return this.fireActorInLoop;
//  }
//
//  public void setFireActorInLoop(boolean fireActorInLoop) {
//      this.fireActorInLoop = fireActorInLoop;
//  }
//
//  public enum TraceTypeEnum {
//      PAJE,
//      VCD
//  }
//
//  @Order(30)
//  @Info("Select the trace format")
//  protected TraceTypeEnum traceType = TraceTypeEnum.PAJE;
//
//  public TraceTypeEnum getTraceType() {
//      return this.traceType;
//  }
//
//  public void setTraceType(TraceTypeEnum traceType) {
//      this.traceType = traceType;
//  }

    @Override
    protected void config() {
        if (useSNGReaderSetup) {
            bind(VPCEvaluator.SimInfo.class).to(SimInfoSNGReader.class);
        } else {
            bind(SimulatorArgumentsImpl.class).toInstance(simulatorArguments);
            bind(SimulatorEnvironmentImpl.class).toInstance(simulatorEnvironment);
            bind(VPCEvaluator.SimInfo.class).to(SimInfoLocal.class);
        }
        bind(VPCEvaluator.VPCObjectives.class).toInstance(objectives);
//      bindConstant(SchedulerType.class).to(schedulerType);
//      bindConstant(TimeSlice.class).to(timeSlice);
//      bindConstant(FireActorInLoop.class).to(fireActorInLoop);
//      bindConstant(TraceType.class).to(traceType);
        bindEvaluator(VPCEvaluator.class);
    }

}
