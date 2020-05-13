package de.fau.scd.VPC.evaluation;

//import static java.lang.annotation.RetentionPolicy.RUNTIME;

//import java.lang.annotation.Retention;

import org.opt4j.core.config.annotations.File;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Required;

import de.fau.scd.Clustering.evaluation.VPCEvaluator.FireActorInLoop;
import de.fau.scd.Clustering.evaluation.VPCEvaluator.NumberOfIterations;
import de.fau.scd.Clustering.evaluation.VPCEvaluator.SchedulerType;
import de.fau.scd.Clustering.evaluation.VPCEvaluator.ApplicationGraph;
import de.fau.scd.Clustering.evaluation.VPCEvaluator.ExecutableOfSimulation;   
import de.fau.scd.Clustering.evaluation.VPCEvaluator.TimeSlice;
import de.fau.scd.Clustering.evaluation.VPCEvaluator.TraceType;
import net.sf.opendse.optimization.evaluator.EvaluatorModule;

public class VPCEvaluatorModule extends EvaluatorModule {
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
    
    public enum TraceTypeEnum{
        PAJE,
        VCD
    }
  
  @Info("Application graph")
  @Order(2)
  @File
  protected String applicationGraph = "";
    
  @Info("The VPC simulation start script.")
  @Order(2)
  @File
  protected String executableOfSimulation = "";

  @Order(0)
  @Info("Set the number of iterations")
  protected int numberOfIterations = 100;
  
  @Order(0)
  @Info("Select the trace format")
  protected TraceTypeEnum traceType = TraceTypeEnum.PAJE;
  
  @Order(0)
  @Info("Choose the scheduler for the VPC simulation")
  protected SchedulerTypeEnum schedulerType = SchedulerTypeEnum.RRNOPRE;
  
  
  @Required(property = "schedulerType", elements = { "RR" })
  protected double timeSlice = 0.00017;
  
  @Required(property = "schedulerType", elements = { "RRNOPRE" })
  protected boolean fireActorInLoop = false;
  
  public String getApplicationGraph() {
      return this.applicationGraph;
  }
  
  public void setApplicationGraph(String applicationGraph) {
      this.applicationGraph = applicationGraph;
  }
  
  public TraceTypeEnum getTraceType() {
      return this.traceType;
  }
  
  public void setTraceType(TraceTypeEnum traceType) {
      this.traceType = traceType;
  }
  
  public boolean getFireActorInLoop() {
      return this.fireActorInLoop;
  }
  
  public void setFireActorInLoop(boolean fireActorInLoop) {
      this.fireActorInLoop = fireActorInLoop;
  }
  
  public double getTimeSlice() {
      return this.timeSlice;
  }
  
  public void setTimeSlice(double timeSlice) {
      this.timeSlice = timeSlice;
  }
  
  public int getNumberOfIterations() {
      return numberOfIterations;
  }
  
  public void setNumberOfIterations(int numberOfIterations) {
      this.numberOfIterations = numberOfIterations; 
  }
  
  public SchedulerTypeEnum getSchedulerType() {
      return schedulerType;
  }
  
  public void setSchedulerType(SchedulerTypeEnum schedulerType) {
      this.schedulerType = schedulerType;
  }
  
  public String getExecutableOfSimulation() {
          return executableOfSimulation;
  }

  public void setExecutableOfSimulation(String executableOfSimulation) {
          this.executableOfSimulation = executableOfSimulation;
  }

  @Override
  protected void config() {
    bindConstant(ExecutableOfSimulation.class).to(executableOfSimulation);
    bindConstant(ApplicationGraph.class).to(applicationGraph);
    bindConstant(NumberOfIterations.class).to(numberOfIterations);
    bindConstant(SchedulerType.class).to(schedulerType);
    bindConstant(TimeSlice.class).to(timeSlice);
    bindConstant(FireActorInLoop.class).to(fireActorInLoop);
    bindConstant(TraceType.class).to(traceType);
    bindEvaluator(VPCEvaluator.class);
  }
    
  
}
