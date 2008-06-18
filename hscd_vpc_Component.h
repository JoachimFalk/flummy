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

#include <vector>
#include <map>
#include <deque>
#include <queue>

namespace SystemC_VPC{

  class ComponentState
  {
    public:
      ComponentState(const size_t &_state) : state(_state) {}

      bool operator==(const ComponentState &rhs) const
      {
        return state == rhs.state;
      }

      bool operator<(const ComponentState &rhs) const
      {
        return state < rhs.state;
      }

    private:
      size_t state;
  };

  /**
   * 
   */
  class PowerMode
  {
    public:
      PowerMode(const size_t &_mode) : mode(_mode) {}

      //FIXME: needed for std::map only
      PowerMode() : mode(0) {}

      bool operator==(const PowerMode &rhs) const
      {
        return mode == rhs.mode;
      }

      bool operator<(const PowerMode &rhs) const
      {
        return mode < rhs.mode;
      }

    private:
      size_t mode;
  };

  class Scheduler;

  typedef std::map<std::string, PowerMode> PowerModes;

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class Component : public AbstractComponent, public ComponentInfo{
    
    SC_HAS_PROCESS(Component);

  public:

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(ProcessControlBlock* pcb);

    
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
      powerMode(NULL),
      powerModes()
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      setScheduler(schedulername);

      powerTable[Component::IDLE]    = 0.0;
      powerTable[Component::RUNNING] = 1.0;

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
    std::deque<ProcessControlBlock*>      newTasks;
    std::map<int,ProcessControlBlock*> readyTasks,runningTasks;
    
    std::map<ComponentState, double> powerTable;
    
    sc_event notify_scheduler_thread;
    sc_signal<trace_value> schedulerTrace;
    
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;

    bool processPower(Attribute att);

    // time last task started
    sc_time startTime;

    void moveToRemainingPipelineStages(ProcessControlBlock* task);
    
    void setScheduler(const char *schedulername);
    
    static const ComponentState IDLE;
    static const ComponentState RUNNING;

    void setComponentState(const ComponentState &state);

    PowerMode *powerMode;

    PowerModes powerModes;

    PowerMode getPowerMode(std::string mode){
      PowerModes::const_iterator i = powerModes.find(mode);
      if(i == powerModes.end()){
        size_t id = powerModes.size();
        powerModes[mode] = PowerMode(id);
      }
      return powerModes[mode];
    }
  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
