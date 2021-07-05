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

package de.fau.scd.VPC.optimization;

import org.opt4j.core.config.annotations.Parent;
import org.opt4j.core.config.annotations.Required;
import org.opt4j.core.problem.ProblemModule;
import org.opt4j.core.start.Constant;
import org.opt4j.viewer.VisualizationModule;

import com.google.inject.multibindings.Multibinder;

//import net.sf.opendse.optimization.DesignSpaceExplorationCreator;
//import net.sf.opendse.optimization.DesignSpaceExplorationDecoder;
import net.sf.opendse.optimization.DesignSpaceExplorationEvaluator;
import net.sf.opendse.optimization.DesignSpaceExplorationModule;
import net.sf.opendse.optimization.ImplementationWidgetService;
import net.sf.opendse.optimization.SpecificationToolBarService;
import net.sf.opendse.optimization.StagnationRestart;
import net.sf.opendse.optimization.constraints.SpecificationConstraints;
import net.sf.opendse.optimization.constraints.SpecificationConstraintsMulti;

@Parent(DesignSpaceExplorationModule.class)
public class OptimizationModularModule extends ProblemModule {

    protected boolean stagnationRestartEnabled = true;

    public boolean isStagnationRestartEnabled() {
        return stagnationRestartEnabled;
    }

    public void setStagnationRestartEnabled(boolean stagnationRestartEnabled) {
            this.stagnationRestartEnabled = stagnationRestartEnabled;
    }

    @Required(property = "stagnationRestartEnabled", elements = { "TRUE" })
    @Constant(value = "maximalNumberStagnatingGenerations", namespace = StagnationRestart.class)
    protected int maximalNumberStagnatingGenerations = 20;

    public int getMaximalNumberStagnatingGenerations() {
            return maximalNumberStagnatingGenerations;
    }

    public void setMaximalNumberStagnatingGenerations(int maximalNumberStagnatingGenerations) {
            this.maximalNumberStagnatingGenerations = maximalNumberStagnatingGenerations;
    }

    /// This is a patched copy of OptimizationNewModule.config
    @Override
    protected void config() {
        // Patch: Modified bindProblem call to use our ModularDSEDecoder.
        bindProblem(ModularDSECreator.class, ModularDSEDecoder.class,
                DesignSpaceExplorationEvaluator.class);
        /* Original was:
         * bindProblem(DesignSpaceExplorationCreator.class, DesignSpaceExplorationDecoder.class,
         *      DesignSpaceExplorationEvaluator.class);
         */

        VisualizationModule.addIndividualMouseListener(binder(), ImplementationWidgetService.class);
        VisualizationModule.addToolBarService(binder(), SpecificationToolBarService.class);

        bind(SpecificationConstraints.class).to(SpecificationConstraintsMulti.class).in(SINGLETON);
        Multibinder.newSetBinder(binder(), SpecificationConstraints.class);

        if (stagnationRestartEnabled) {
            addOptimizerIterationListener(StagnationRestart.class);
        }

    }

}
