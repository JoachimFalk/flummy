package de.cs12.dsevpc.evaluator;

import org.opt4j.core.config.annotations.File;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;

import de.cs12.dse.optimization.AbstractEvaluatorModule;
import de.cs12.dsevpc.evaluator.VPCEvaluator.StartScript;

public class VPCEvaluatorModule extends AbstractEvaluatorModule {

  @Info("The VPC simulation start script.")
  @Order(1)
  @File
  protected String startScript = "";

  
  
  @Override
  protected void config() {
    bindConstant(StartScript.class).to(startScript);
    addEvaluator(VPCEvaluator.class);
  }
    

  public String getStartScript() {
          return startScript;
  }

  public void setStartScript(String startScript) {
          this.startScript = startScript;
  }

}
