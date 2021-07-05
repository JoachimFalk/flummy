// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2018 FAU -- Fedor Smirnov <fedor.smirnov@fau.de>
 *   2019 FAU -- Fedor Smirnov <fedor.smirnov@fau.de>
 *   2020 FAU -- Fedor Smirnov <fedor.smirnov@fau.de>
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

package de.fau.scd.VPC.decoders;

import net.sf.opendse.encoding.ImplementationEncodingModularDefault;
import net.sf.opendse.encoding.allocation.AllocationEncodingNone;
import net.sf.opendse.encoding.allocation.AllocationEncodingUtilization;
import net.sf.opendse.encoding.AllocationEncoding;
import net.sf.opendse.encoding.ImplementationEncodingModular;
import net.sf.opendse.encoding.interpreter.InterpreterVariable;
import net.sf.opendse.encoding.interpreter.SpecificationPostProcessorCycleRemover;
import net.sf.opendse.encoding.module.VariableClassOrderModular;
import net.sf.opendse.encoding.routing.CycleBreakEncoder;
import net.sf.opendse.encoding.routing.CycleBreakEncoderNone;
import net.sf.opendse.encoding.routing.RoutingEncodingFlexible;
import net.sf.opendse.encoding.routing.RoutingEncodingNone;
import net.sf.opendse.optimization.ImplementationEvaluator;
import net.sf.opendse.optimization.RoutingVariableClassOrder;
import net.sf.opendse.optimization.SATConstraints;
import net.sf.opendse.optimization.SATCreatorDecoder;
import net.sf.opendse.optimization.constraints.SpecificationCapacityConstraints;
import net.sf.opendse.optimization.constraints.SpecificationConnectConstraints;
import net.sf.opendse.optimization.constraints.SpecificationConstraints;
import net.sf.opendse.optimization.constraints.SpecificationConstraintsMulti;
import net.sf.opendse.optimization.constraints.SpecificationElementsConstraints;
import net.sf.opendse.optimization.constraints.SpecificationRouterConstraints;
import net.sf.opendse.optimization.encoding.Encoding;
import net.sf.opendse.optimization.encoding.Encoding.RoutingEncoding;
import net.sf.opendse.optimization.encoding.ImplementationEncoding;
import net.sf.opendse.optimization.encoding.Interpreter;
import net.sf.opendse.optimization.encoding.InterpreterSpecification;

import de.fau.scd.VPC.optimization.DecoderModule;

import org.opt4j.core.start.Constant;

import com.google.inject.multibindings.Multibinder;

//@Parent(DesignSpaceExplorationModule.class)
public class SATDecoderModule extends DecoderModule {

    /**
     * Different allocation encodings: NONE: Allocation not encoded, see
     * {@link AllocationEncodingNone} UTILIZATION: Everything that is used for
     * either tasks or messages allocated, see
     * {@link AllocationEncodingUtilization} CUSTOM: Allocation encoding chosen
     * in a different module (e.g. outside the opendse projects)
     *
     * @author Fedor Smirnov
     *
     */
    protected enum ChosenAllocationEncoding {
        NONE, UTILIZATION, CUSTOM
    }

    /**
     * Different routing encodings: NONE: Routing not encoded, see
     * {@link RoutingEncodingNone} Flexible: Rather complex scheme, see
     * {@link AllocationEncodingUtilization} CUSTOM: Routing encoding chosen in
     * a different module (e.g. outside the opendse projects)
     *
     * @author Fedor Smirnov
     *
     */
    protected enum ChosenRoutingEncoding {
        NONE, FLEXIBLE, CUSTOM
    }

    protected ChosenAllocationEncoding allocationEncoding = ChosenAllocationEncoding.UTILIZATION;

    protected ChosenRoutingEncoding routingEncodingType = ChosenRoutingEncoding.FLEXIBLE;

    protected RoutingEncoding routingEncoding = RoutingEncoding.FLOW;

    @Constant(value = "preprocessing", namespace = SATConstraints.class)
    protected boolean usePreprocessing = true;

    protected boolean useModularEncoding = false;

    protected boolean removeCyclesManually = false;

    @Constant(value = "variableorder", namespace = SATCreatorDecoder.class)
    protected boolean useVariableOrder = true;

    public boolean isRemoveCyclesManually() {
        return removeCyclesManually;
    }

    public void setRemoveCyclesManually(boolean removeCyclesManually) {
        this.removeCyclesManually = removeCyclesManually;
    }

    public boolean isUseModularEncoding() {
        return useModularEncoding;
    }

    public void setUseModularEncoding(boolean useModularEncoding) {
        this.useModularEncoding = useModularEncoding;
    }

    public RoutingEncoding getRoutingEncoding() {
        return routingEncoding;
    }

    public void setRoutingEncoding(RoutingEncoding routingEncoding) {
        this.routingEncoding = routingEncoding;
    }

    public boolean isUsePreprocessing() {
        return usePreprocessing;
    }

    public void setUsePreprocessing(boolean usePreprocessing) {
        this.usePreprocessing = usePreprocessing;
    }

    public boolean isUseVariableOrder() {
        return useVariableOrder;
    }

    public void setUseVariableOrder(boolean useVariableOrder) {
        this.useVariableOrder = useVariableOrder;
    }

    public ChosenAllocationEncoding getAllocationEncoding() {
        return allocationEncoding;
    }

    public void setAllocationEncoding(ChosenAllocationEncoding allocationEncoding) {
        this.allocationEncoding = allocationEncoding;
    }

    public ChosenRoutingEncoding getRoutingEncodingType() {
        return routingEncodingType;
    }

    public void setRoutingEncodingType(ChosenRoutingEncoding routingEncodingType) {
        this.routingEncodingType = routingEncodingType;
    }

    @Override
    protected void config() {
        bindCreatorDecoder(SATDecoder.class);

        bind(SpecificationConstraints.class).to(SpecificationConstraintsMulti.class).in(SINGLETON);
        Multibinder<SpecificationConstraints> scmulti = Multibinder.newSetBinder(binder(),
                SpecificationConstraints.class);
        scmulti.addBinding().to(SpecificationCapacityConstraints.class);
        scmulti.addBinding().to(SpecificationConnectConstraints.class);
        scmulti.addBinding().to(SpecificationElementsConstraints.class);
        scmulti.addBinding().to(SpecificationRouterConstraints.class);

        Multibinder.newSetBinder(binder(), ImplementationEvaluator.class);

        if (!useModularEncoding) {
            bind(RoutingEncoding.class).toInstance(routingEncoding);
            if (useVariableOrder) {
                bind(RoutingVariableClassOrder.class).asEagerSingleton();
            }
            bind(Interpreter.class).to(InterpreterSpecification.class);
            bind(ImplementationEncoding.class).to(Encoding.class);
        } else {
            bind(Interpreter.class).to(InterpreterVariable.class);
            bind(ImplementationEncoding.class).to(ImplementationEncodingModular.class);
            bind(ImplementationEncodingModular.class).to(ImplementationEncodingModularDefault.class);
            if (useVariableOrder) {
                bind(VariableClassOrderModular.class).asEagerSingleton();
            }
        }

        if (removeCyclesManually) {
            bind(CycleBreakEncoder.class).to(CycleBreakEncoderNone.class);
            bind(SpecificationPostProcessorCycleRemover.class).asEagerSingleton();
        }

        // bind the allocation encoder
        switch (allocationEncoding) {
        case UTILIZATION:
            bind(AllocationEncoding.class).to(AllocationEncodingUtilization.class);
            break;

        case NONE:
            bind(AllocationEncoding.class).to(AllocationEncodingNone.class);

        case CUSTOM:
            // do nothing, defined elsewhere
            break;

        default:
            break;
        }

        // bind the routing encoder
        switch (routingEncodingType) {
        case NONE:
            bind(net.sf.opendse.encoding.RoutingEncoding.class).to(RoutingEncodingNone.class);
            break;

        case FLEXIBLE:
            bind(net.sf.opendse.encoding.RoutingEncoding.class).to(RoutingEncodingFlexible.class);

        case CUSTOM:
            // no binding, done elsewhere
            break;
        default:
            break;
        }
    }
}
