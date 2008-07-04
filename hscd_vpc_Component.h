/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_Component.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include <systemc.h>

#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_AbstractComponent.h"
#include "ComponentInfo.h"
#include "PowerSumming.h"
#include "PowerMode.h"

#include <vector>
#include <map>
#include <deque>
#include <queue>

namespace SystemC_VPC{

  class Scheduler;

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class Component : public AbstractComponent{
    
    SC_HAS_PROCESS(Component);

  public:

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(Task* task);

    
    /**
     * \brief Used to create the Tracefiles.
     *
     * To create a vcd-trace-file in SystemC all the signals to 
     * trace have to be in a "global" scope. The signals have to 
     * be created in elaboration phase (before first sc_start).
     */
    virtual void informAboutMapping(std::string module);
      
    /**
     * \brief An implementation of AbstractComponent used together with
     * passive actors and global SMoC v2 Schedulers.
     */
    Component( sc_module_name name,
               const char *schedulername )
      : AbstractComponent(name),
      powerMode(NULL)
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      this->setPowerMode(this->translatePowerMode("FAST"));
      setScheduler(schedulername);

      powerTable[ComponentState::IDLE]    = 0.0;
      powerTable[ComponentState::RUNNING] = 1.0;

      std::string powerSumFileName(this->getName());
      powerSumFileName += ".dat";

      powerSumStream = new std::ofstream(powerSumFileName.c_str());
      powerSumming   = new PowerSumming(*powerSumStream);
      this->addObserver(powerSumming);

#ifndef NO_VCD_TRACES
      std::string tracefilename=this->getName(); //componentName;
      char tracefilechar[VPC_MAX_STRING_LENGTH];
      char* traceprefix= getenv("VPCTRACEFILEPREFIX");
      if(0!=traceprefix){
        tracefilename.insert(0,traceprefix);
      }
      strcpy(tracefilechar,tracefilename.c_str());
      vcd_trace_file *vcd = new vcd_trace_file(tracefilechar);
      //sc_create_vcd_trace_file(tracefilechar);
      this->traceFile = vcd;
      sc_get_curr_simcontext()->add_trace_file(this->traceFile);
      vcd->sc_set_vcd_time_unit(-9);

      // FIXME: disabled Scheduler tracing (there is no scheduling overhead)
      //sc_trace(this->traceFile,schedulerTrace,schedulername);
#endif //NO_VCD_TRACES      
    }
      
    virtual ~Component()
    {
      this->removeObserver(powerSumming);
      delete powerSumming;
      delete powerSumStream;
    }

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void processAndForwardParameter(char *sType,char *sValue);
    virtual void setAttribute(Attribute& fr_Attributes);
    
  protected:

    virtual void schedule_thread(); 

    virtual void remainingPipelineStages(); 

  private:
    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair, std::vector<timePcbPair>,timeCompare> pqueue;

    sc_trace_file *traceFile;
    std::map<std::string, Tracing* > trace_map_by_name;
    Scheduler *scheduler;
    std::deque<Task*>      newTasks;
    TaskMap readyTasks,runningTasks;
    
    std::map<ComponentState, double> powerTable;
    
    sc_event notify_scheduler_thread;
    sc_signal<trace_value> schedulerTrace;
    
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;

    bool processPower(Attribute att);

    // time last task started
    sc_time startTime;

    void moveToRemainingPipelineStages(Task* task);
    
    void setScheduler(const char *schedulername);
    
    void setComponentState(const ComponentState &state);

    PowerMode *powerMode;
  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
