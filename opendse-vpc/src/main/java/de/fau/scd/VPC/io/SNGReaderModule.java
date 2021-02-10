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
package de.fau.scd.VPC.io;

import de.fau.scd.VPC.config.annotations.Text;
import de.fau.scd.VPC.config.properties.Arguments;
import de.fau.scd.VPC.config.properties.Environment;
import de.fau.scd.VPC.config.visualization.PropertyPanel;
import de.fau.scd.VPC.io.SNGImporter.FIFOTranslation;
import de.fau.scd.VPC.io.SpecificationWrapperSNG.DFGSource;
import de.fau.scd.VPC.io.SpecificationWrapperSNG;

import net.sf.opendse.optimization.SpecificationWrapper;
import net.sf.opendse.optimization.io.IOModule;

import org.opt4j.core.config.annotations.File;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Panel;
import org.opt4j.core.config.annotations.Required;
import org.opt4j.core.start.Constant;

@Panel(value = PropertyPanel.class)
public class SNGReaderModule extends IOModule {
    
    @Info("Select how the dataflow graph is derived, i.e., from an SNG file (DFG_FROM_SNG_FILE) or exported from the SysteMoC virtual prototype (DFG_FROM_SIM_EXPORT).")
    @Order(0)
    @Constant(value = "dfgSource", namespace = SpecificationWrapperSNG.class)
    protected DFGSource dfgSource = DFGSource.DFG_FROM_SNG_FILE;

    public DFGSource getDfgSource() {
        return dfgSource;
    }

    public void setDfgSource(DFGSource dfgSource) {
        this.dfgSource = dfgSource;
    }

    @Info("The dataflow graph given as an SNG XML file.")
    @Order(1)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "sngFile")
    @Required(property = "dfgSource", elements = { "DFG_FROM_SNG_FILE" })
    @File
    protected String sngFile = "";

    public String getSngFile() {
        return sngFile;
    }

    public void setSngFile(String sngFile) {
        this.sngFile = sngFile;
    }

    @Info("The dataflow graph will be exported from the given SysteMoC virtual prototype.")
    @Order(1)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "simulatorExecutable")
    @Required(property = "dfgSource", elements = { "DFG_FROM_SIM_EXPORT" })
    @File
    protected String simulatorExecutable = "";

    public String getSimulatorExecutable() {
        return simulatorExecutable;
    }

    public void setSimulatorExecutable(String simulatorExecutable) {
        this.simulatorExecutable = simulatorExecutable;
    }
    
    protected static class SimulatorArgumentsImpl
        extends
            Arguments
        implements
            SpecificationWrapperSNG.SimulatorArguments
//        , VPCEvaluator.SimulatorArguments
    {
    }
    
    @Info("Arguments for the SysteMoC virtual prototype.")
    @Order(2)
    @Required(property = "dfgSource", elements = { "DFG_FROM_SIM_EXPORT" })
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
            SpecificationWrapperSNG.SimulatorEnvironment
    {
    }
    
    @Info("Environment for the SysteMoC virtual prototype.")
    @Order(3)
    @Required(property = "dfgSource", elements = { "DFG_FROM_SIM_EXPORT" })
    protected final SimulatorEnvironmentImpl simulatorEnvironment = new SimulatorEnvironmentImpl();

    public Environment getSimulatorEnvironment() {
        return simulatorEnvironment;
    }

    public void setSimulatorEnvironment(Environment simulatorEnvironment) {
        this.simulatorEnvironment.clear();
        this.simulatorEnvironment.putAll(simulatorEnvironment);
    }

    @Info("The architecture given as VPC configuration XML template file.")
    @Order(10)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "vpcConfigTemplate")
    @File
    protected String vpcConfigTemplate = "";

    public String getVpcConfigTemplate() {
        return vpcConfigTemplate;
    }

    public void setVpcConfigTemplate(String vpcConfigTemplate) {
        this.vpcConfigTemplate = vpcConfigTemplate;
    }

    @Info("Select how FIFOs are translated into the DSE model.")
    @Order(20)
    @Constant(value = "fifoTranslation", namespace = SpecificationWrapperSNG.class)
    protected FIFOTranslation fifoTranslation = FIFOTranslation.FIFO_IS_MEMORY_TASK;

    public FIFOTranslation getFifoTranslation() {
        return fifoTranslation;
    }

    public void setFifoTranslation(FIFOTranslation fifoTranslation) {
        this.fifoTranslation = fifoTranslation;
    }

    @Info("If true, multicast communication is generated for FIFOs into which identical data is written.")
    @Order(21)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "multicastMessages")
    @Required(property = "fifoTranslation", elements = { "FIFO_IS_MESSAGE" })
    protected boolean multicastMessages = true;

    public boolean getMulticastMessages() {
        return multicastMessages;
    }

    public void setMulticastMessages(boolean multicastMessages) {
        this.multicastMessages = multicastMessages;
    }

    @Info("If true, only one memory task is generated for FIFOs into which identical data is written.")
    @Order(21)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "shareFIFOBuffers")
    @Required(property = "fifoTranslation", elements = { "FIFO_IS_MEMORY_TASK" })
    protected boolean shareFIFOBuffers = true;

    public boolean getShareFIFOBuffers() {
        return shareFIFOBuffers;
    }

    public void setShareFIFOBuffers(boolean shareFIFOBuffers) {
        this.shareFIFOBuffers = shareFIFOBuffers;
    }

    @Override
    protected void config() {
        bind(SpecificationWrapper.class).to(SpecificationWrapperSNG.class).asEagerSingleton();//in(SINGLETON);
        bind(SpecificationWrapperSNG.SimulatorEnvironment.class).toInstance(simulatorEnvironment);
        bind(SpecificationWrapperSNG.SimulatorArguments.class).toInstance(simulatorArguments);
    }

}
