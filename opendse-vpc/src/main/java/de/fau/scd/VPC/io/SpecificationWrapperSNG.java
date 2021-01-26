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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.VPC.helper.TempDirectoryHandler;
import de.fau.scd.VPC.io.Common.FormatErrorException;
import de.fau.scd.VPC.io.SNGImporter.FIFOTranslation;
import de.fau.scd.VPC.config.properties.Environment;

import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mappings;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.SpecificationWrapper;
import net.sf.opendse.optimization.io.SpecificationTransformer;
import net.sf.opendse.optimization.io.SpecificationWrapperInstance;

public class SpecificationWrapperSNG implements SpecificationWrapper {
    
    public enum DFGSource {
        DFG_FROM_SNG_FILE
      , DFG_FROM_SIM_EXPORT
    };
    
    public interface SimulatorEnvironment {
        public String [] getEnvironment();
    }

    final private SpecificationWrapperInstance specificationWrapperInstance;
    
    private enum QuoteState {
        ARGUMENT_SEPERATOR, SINGLE_QUOTE, DOUBLE_QUOTE, NO_QUOTE
    };

    @Inject
    public SpecificationWrapperSNG(
        @Constant(namespace = SpecificationWrapperSNG.class, value = "dfgSource")           DFGSource dfgSource
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "sngFile")             String sngFileName
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "simulatorExecutable") String simulatorExecutable
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "simulatorArguments")  String simulatorArguments
      , SimulatorEnvironment                                                                       simulatorEnvironment
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "vpcConfigTemplate")   String vpcConfigTemplate
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "fifoTranslation")     FIFOTranslation fifoTranslation
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "multicastMessages")   boolean multicastMessages
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "shareFIFOBuffers")    boolean shareFIFOBuffers
        ) throws IOException, FileNotFoundException, FormatErrorException
    {
        UniquePool uniquePool = new UniquePool();

        SNGReader sngReader = null;
        switch (dfgSource) {
        case DFG_FROM_SNG_FILE:
            sngReader = new SNGReader(sngFileName);
            break;
        case DFG_FROM_SIM_EXPORT:
            TempDirectoryHandler tempDirectoryHandler =
                TempDirectoryHandler.getTempDirectoryHandler();
            File sngFile = Files.createTempFile(
                    tempDirectoryHandler.getDirectory().toPath()
                  , "dfg", ".sng").toFile();
            
            try {
                ArrayList<String> cmd = new ArrayList<String>();
                cmd.add(simulatorExecutable);
                cmd.add("--systemoc-export-sng");
                cmd.add(sngFile.getAbsolutePath());
                
                {
                    QuoteState quoteState = QuoteState.ARGUMENT_SEPERATOR;
                    
                    StringBuilder argument = null;new StringBuilder();

                    for (char c : simulatorArguments.toCharArray()) {
                        switch (quoteState) {
                        case ARGUMENT_SEPERATOR:
                            if (c == '"') {
                                quoteState = QuoteState.DOUBLE_QUOTE;
                                argument = new StringBuilder();
                            } else if (c == '\'') {
                                quoteState = QuoteState.SINGLE_QUOTE;
                                argument = new StringBuilder();
                            } else if (Character.isWhitespace(c)) {
                                quoteState = QuoteState.ARGUMENT_SEPERATOR;
                            } else {
                                quoteState = QuoteState.NO_QUOTE;
                                argument = new StringBuilder();                                
                                argument.append(c);
                            }                            
                            break;
                        case NO_QUOTE:
                            if (c == '"') {
                                quoteState = QuoteState.DOUBLE_QUOTE;
                            } else if (c == '\'') {
                                quoteState = QuoteState.SINGLE_QUOTE;
                            } else if (Character.isWhitespace(c)) {
                                quoteState = QuoteState.ARGUMENT_SEPERATOR;
                                cmd.add(argument.toString());
                                argument = null;
                            } else {
                                quoteState = QuoteState.NO_QUOTE;
                                argument.append(c);                                
                            }                            
                            break;
                        case SINGLE_QUOTE:
                            break;
                        case DOUBLE_QUOTE:
                            break;
                        }
                    }
                    if (argument != null) {
                        cmd.add(argument.toString());
                        argument = null;                        
                    }                   
                }
                System.out.println(cmd.toString());
                Process exec_process = Runtime.getRuntime().exec(
                    cmd.toArray(new String[0]), simulatorEnvironment.getEnvironment(),
                    tempDirectoryHandler.getDirectory());
                int status = exec_process.waitFor();
                if (status != 0) {
                    sngFile.delete();
                    sngFile = null;
                }
            } catch (IOException e) {
                System.err.println("Got an exception during DFG export from simulator:\n" + e);
                sngFile.delete();
                sngFile = null;
            } catch (InterruptedException e) {
                System.err.println("Got an exception during DFG export from simulator:\n" + e);
                sngFile.delete();
                sngFile = null;
            }            
            sngReader = new SNGReader(sngFile);
            break;
        }

        Boolean generateMulticast = null;
        switch (fifoTranslation) {
        case FIFO_IS_MESSAGE:
            generateMulticast = multicastMessages;
            break;
        case FIFO_IS_MEMORY_TASK:
            generateMulticast = shareFIFOBuffers;
            break;
        }
        assert generateMulticast != null : "Oops, internal error!";

        SNGImporter sngImporter = new SNGImporter(sngReader, uniquePool, fifoTranslation, generateMulticast);
        Application<Task, Dependency> application = sngImporter.getApplication();

        VPCConfigReader vpcConfigReader = new VPCConfigReader(vpcConfigTemplate);
        VPCConfigImporter vpcConfigImporter = new VPCConfigImporter(vpcConfigReader,uniquePool, application);
        Architecture<Resource, Link> architecture = vpcConfigImporter.getArchitecture();
        Mappings<Task, Resource> mappings = vpcConfigImporter.getMappings();

        Specification specification = new Specification(application, architecture, mappings);
        specificationWrapperInstance = new SpecificationWrapperInstance(specification);
    }

    @Override
    public Specification getSpecification() {
        return specificationWrapperInstance.getSpecification();
    }

    @Inject(optional = true)
    public void setSpecificationTransformers(Set<SpecificationTransformer> transformers) {
        specificationWrapperInstance.setSpecificationTransformers(transformers);
    }

}
