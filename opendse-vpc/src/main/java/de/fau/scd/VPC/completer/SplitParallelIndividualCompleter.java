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

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.opt4j.core.Genotype;
import org.opt4j.core.Individual;
import org.opt4j.core.common.completer.SequentialIndividualCompleter;
import org.opt4j.core.optimizer.Control;
import org.opt4j.core.optimizer.Optimizer;
import org.opt4j.core.optimizer.OptimizerStateListener;
import org.opt4j.core.optimizer.TerminationException;
import org.opt4j.core.problem.Decoder;
import org.opt4j.core.problem.Evaluator;
import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

/**
 * The {@link SplitParallelIndividualCompleter} completes {@link Individual}s
 * with multiple threads.
 * 
 * @author glass, schwarzer
 * 
 */
public class SplitParallelIndividualCompleter extends SequentialIndividualCompleter implements OptimizerStateListener {
    protected final ExecutorService decodeExecutor;
    protected final ExecutorService evaluateExecutor;

    /**
     * The {@link DecodeComplete} class completes a single {@link Individual} to
     * the state {@link Individual.State.PHENOTYPED}.
     * 
     * @author glass, schwarzer
     * 
     */
    protected class DecodeComplete implements Callable<Void> {

        protected final Individual individual;

        protected final Control control;

        /**
         * Constructs {@link DecodeComplete} with an {@link Individual}.
         * 
         * @param individual
         *            the individual to complete
         * @param control
         *            the control
         */
        public DecodeComplete(final Individual individual, final Control control) {
            this.individual = individual;
            this.control = control;
        }

        /*
         * (non-Javadoc)
         * 
         * @see java.util.concurrent.Callable#call()
         */
        @Override
        public Void call() throws TerminationException {
            if (!individual.isEvaluated()) {
                control.checkpoint();
                SplitParallelIndividualCompleter.this.decode(individual);
                control.checkpoint();
            }
            return null;
        }
    }

    /**
     * The {@link EvaluateComplete} class completes a single {@link Individual}
     * to the state {@link Individual.State.EVALUATED}.
     * 
     * @author glass, schwarzer
     * 
     */
    protected class EvaluateComplete implements Callable<Void> {

        protected final Individual individual;

        protected final Control control;

        /**
         * Constructs {@link EvaluateComplete} with an {@link Individual}.
         * 
         * @param individual
         *            the individual to complete
         * @param control
         *            the control
         */
        public EvaluateComplete(final Individual individual, final Control control) {
            this.individual = individual;
            this.control = control;
        }

        /*
         * (non-Javadoc)
         * 
         * @see java.util.concurrent.Callable#call()
         */
        @Override
        public Void call() throws TerminationException {
            if (!individual.isEvaluated()) {
                control.checkpoint();
                SplitParallelIndividualCompleter.this.evaluate(individual);
                control.checkpoint();
            }
            return null;
        }
    }

    /**
     * Constructs a {@link SplitParallelIndividualCompleter} with a specified
     * maximal number of concurrent threads.
     * 
     * @param control
     *            the control
     * @param decoder
     *            the decoder
     * @param evaluator
     *            the evaluator
     * @param maxDecodeThreads
     *            the maximal number of parallel threads for decoding (using
     *            namespace {@link SplitParallelIndividualCompleter})
     * @param maxEvaluateThreads
     *            the maximal number of parallel threads for evaluation (using
     *            namespace {@link SplitParallelIndividualCompleter})
     */
    @Inject
    public SplitParallelIndividualCompleter(Control control, Decoder<Genotype, Object> decoder,
            Evaluator<Object> evaluator,
            @Constant(value = "maxDecodeThreads", namespace = SplitParallelIndividualCompleter.class) int maxDecodeThreads,
            @Constant(value = "maxEvaluateThreads", namespace = SplitParallelIndividualCompleter.class) int maxEvaluateThreads) {
        super(control, decoder, evaluator);

        if (maxDecodeThreads < 1 || maxEvaluateThreads < 1) {
            throw new IllegalArgumentException(
                    "Invalid number of threads: Decode: " + maxDecodeThreads + ", Evaluate: " + maxEvaluateThreads);
        }
        this.decodeExecutor = Executors.newFixedThreadPool(maxDecodeThreads);
        this.evaluateExecutor = Executors.newFixedThreadPool(maxEvaluateThreads);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.opt4j.core.common.completer.SequentialIndividualCompleter#complete
     * (java.lang.Iterable)
     */
    @Override
    public void complete(Iterable<? extends Individual> iterable) throws TerminationException {

        try {
            List<Future<Void>> returns = new ArrayList<Future<Void>>();

            for (Individual individual : iterable) {
                if (individual.getState() != Individual.State.PHENOTYPED) {
                    returns.add(decodeExecutor.submit(new DecodeComplete(individual, control)));
                }
            }

            for (Future<Void> future : returns) {
                try {
                    future.get();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        } catch (ExecutionException ex) {
            decodeExecutor.shutdownNow();
            if (ex.getCause() instanceof TerminationException) {
                throw (TerminationException) ex.getCause();
            }
            throw new RuntimeException(ex);
        }

        try {
            List<Future<Void>> returns = new ArrayList<Future<Void>>();

            for (Individual individual : iterable) {
                if (individual.getState() != Individual.State.EVALUATED) {
                    returns.add(evaluateExecutor.submit(new EvaluateComplete(individual, control)));
                }
            }

            for (Future<Void> future : returns) {
                try {
                    future.get();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        } catch (ExecutionException ex) {
            evaluateExecutor.shutdownNow();
            if (ex.getCause() instanceof TerminationException) {
                throw (TerminationException) ex.getCause();
            }
            throw new RuntimeException(ex);
        }

    }

    /*
     * (non-Javadoc)
     * 
     * @see java.lang.Object#finalize()
     */
    @Override
    protected void finalize() throws Throwable {
        shutdownExecutorService();
        super.finalize();
    }

    /**
     * Shutdown the {@link ExecutorService}.
     */
    protected synchronized void shutdownExecutorService() {
        if (!decodeExecutor.isShutdown()) {
            decodeExecutor.shutdown();
        }
        if (!evaluateExecutor.isShutdown()) {
            evaluateExecutor.shutdown();
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.opt4j.core.optimizer.OptimizerStateListener#optimizationStarted(org
     * .opt4j.core.optimizer.Optimizer)
     */
    @Override
    public void optimizationStarted(Optimizer optimizer) {
        // do nothing
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.opt4j.core.optimizer.OptimizerStateListener#optimizationStopped(org
     * .opt4j.core.optimizer.Optimizer)
     */
    @Override
    public void optimizationStopped(Optimizer optimizer) {
        shutdownExecutorService();
    }
}
