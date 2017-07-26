package de.cs12.dsevpc.evaluator;

import de.cs12.dse.optimization.AbstractEvaluatorModule;


public class VPCEvaluatorModule extends AbstractEvaluatorModule {

  @Override
  protected void config() {
    addEvaluator(VPCEvaluator.class);
  }
    
}
