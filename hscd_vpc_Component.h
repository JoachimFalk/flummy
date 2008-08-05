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
#include "hscd_vpc_Director.h"

#include <vector>
#include <map>
#include <deque>
#include <queue>

namespace SystemC_VPC{

  class Scheduler;

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<PowerMode, PowerTable>  PowerTables;

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
     *
     */
    virtual void requestBlockingCompute(Task* task, Event* blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Event* blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Event* blocker);
    
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
               const char *schedulername,
               Director *director )
      : AbstractComponent(name),
        blockMutex(0)
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      this->setPowerMode(this->translatePowerMode("FAST"));
      setScheduler(schedulername);

      midPowerGov = new LoadHysteresisGovernor(
        director->topPowerGov,
        sc_time(20,  SC_MS),
        sc_time(10, SC_MS),
        sc_time(2, SC_MS));
      this->addObserver(midPowerGov);

      if(powerTables.find(*getPowerMode()) == powerTables.end()){
        powerTables[*getPowerMode()] = PowerTable();
      }

      PowerTable &powerTable=powerTables[*getPowerMode()];
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
#ifndef NO_VCD_TRACES
      sc_close_vcd_trace_file(traceFile);
#endif //NO_VCD_TRACES      
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

#ifndef NO_VCD_TRACES
    sc_trace_file *traceFile;
    std::map<std::string, Tracing* > trace_map_by_name;
    sc_signal<trace_value> schedulerTrace;
#endif //NO_VCD_TRACES      

    Scheduler *scheduler;
    std::deque<Task*>      newTasks;
    
    PowerTables powerTables;
    
    sc_event notify_scheduler_thread;
    Event blockCompute;
    size_t   blockMutex;
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;

    LoadHysteresisGovernor *midPowerGov;

    bool processPower(Attribute att);

    // time last task started
    sc_time startTime;

    void moveToRemainingPipelineStages(Task* task);
    
    void setScheduler(const char *schedulername);
    
    void fireStateChanged(const ComponentState &state);

    void addTasks();
  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
