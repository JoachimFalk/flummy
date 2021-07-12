// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2014 FAU -- Michael Glass <michael.glass@cs.fau.de>
 *   2014 FAU -- Tobias Schwarzer <tobias.schwarzer@fau.de>
 *   2018 FAU -- Martin Letras <martin.letras@fau.de>
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

package de.fau.scd.VPC.completer;

import org.opt4j.core.config.Icons;
import org.opt4j.core.config.annotations.Icon;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.optimizer.IndividualCompleter;
import org.opt4j.core.start.Constant;
import org.opt4j.core.start.Opt4JModule;

/**
 * The {@link SplitIndividualCompleterModule} is used to choose and configure a
 * {@link IndividualCompleter}.
 * 
 * @author glass, schwarzer
 * 
 */
@Icon(Icons.PUZZLE_BLUE)
@Info("The IndividualCompleter decodes and evaluates the individuals in the optimization process.")
public class SplitIndividualCompleterModule extends Opt4JModule {

    @Info("Sets the number of parallel decode processes.")
    @Constant(value = "maxDecodeThreads", namespace = SplitParallelIndividualCompleter.class)
    protected int decodeThreads = 4;

    @Info("Sets the number of parallel evaluate processes.")
    @Constant(value = "maxEvaluateThreads", namespace = SplitParallelIndividualCompleter.class)
    protected int evaluateThreads = 4;

    /**
     * Returns the maximal number of parallel decode threads.
     * 
     * @see #setDecodeThreads
     * @return the maximal number of parallel decode threads
     */
    public int getDecodeThreads() {
        return decodeThreads;
    }

    /**
     * Sets the maximal number of parallel decode threads.
     * 
     * @see #getDecodeThreads
     * @param decodeThreads
     *            the maximal number of parallel decode threads
     */
    public void setDecodeThreads(int decodeThreads) {
        if (decodeThreads <= 0) {
            throw new IllegalArgumentException("The number of decode threads must be positive: " + decodeThreads);
        }
        this.decodeThreads = decodeThreads;
    }

    /**
     * Returns the maximal number of parallel evaluate threads.
     * 
     * @see #setEvaluateThreads
     * @return the maximal number of parallel evaluate threads
     */
    public int getEvaluateThreads() {
        return evaluateThreads;
    }

    /**
     * Sets the maximal number of parallel evaluate threads.
     * 
     * @see #getEvaluateThreads
     * @param evaluateThreads
     *            the maximal number of parallel evaluate threads
     */
    public void setEvaluateThreads(int evaluateThreads) {
        if (evaluateThreads <= 0) {
            throw new IllegalArgumentException("The number of evaluate threads must be positive: " + evaluateThreads);
        }
        this.evaluateThreads = evaluateThreads;
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.opt4j.start.Opt4JModule#config()
     */
    @Override
    public void config() {
        bind(SplitParallelIndividualCompleter.class).in(SINGLETON);
        bind(IndividualCompleter.class).to(SplitParallelIndividualCompleter.class);
        addOptimizerStateListener(SplitParallelIndividualCompleter.class);
    }
}
