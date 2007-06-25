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

#include <vector.h>
#include <map.h>
#include <deque.h>
#include <queue.h>

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
     * \brief Preempts execution of component
     * Used to deallocate the current execution of a component.
     * \sa AbstractComponent::deallocate
     */
    virtual void deallocate(bool kill);

    /**
     * \brief Resumes deallocated execution
     * Used to allocate execution of deallocated component.
     * \sa AbstractComponent
     */
    virtual void allocate();

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
      : AbstractComponent(name)
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      setScheduler(schedulername);

#ifndef NO_VCD_TRACES
      std::string tracefilename=this->basename(); //componentName;
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

          /**************************/
          /*  EXTENSION SECTION     */
          /**************************/
      if(!this->activ){
#ifdef VPC_DEBUG
	std::cerr << VPC_GREEN(this->basename() << "> Activating")
		  << std::endl;
#endif //VPC_DEBUG
	this->setActiv(true);
      }
          /**************************/
          /*  END OF EXTENSION      */
          /**************************/
    }
      
    virtual ~Component(){}

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void processAndForwardParameter(char *sType,char *sValue);
    
  protected:

    /**
     * implementation of AbstractComponent::compute(const char *, const char *,
     * VPC_Event)
     */
    virtual void _compute( const char *name,
                           const char *funcname=NULL,
                           VPC_Event *end=NULL )
      __attribute__ ((deprecated));
    
    /**
     * implementation of AbstractComponent::compute(const char *, VPC_Event)
     */
    virtual void _compute( const char *name, VPC_Event *end=NULL)
      __attribute__ ((deprecated));
    
    virtual void schedule_thread(); 

    virtual void remainingPipelineStages(); 

    // used to indicate deallocation request
    sc_event notify_deallocate;

    // used to indicate allocate request
    sc_event notify_allocate;

  private:
    sc_event remainingPipelineStages_WakeUp;
    priority_queue<timePcbPair, vector<timePcbPair>,timeCompare> pqueue;

    sc_trace_file *traceFile;
    map<std::string, Tracing* > trace_map_by_name;
    Scheduler *scheduler;
    deque<ProcessControlBlock*>      newTasks;
    map<int,ProcessControlBlock*> readyTasks,runningTasks;
    
    sc_event notify_scheduler_thread;
    sc_signal<trace_value> schedulerTrace;
    
    inline void resignTask( int &taskToResign,
                            sc_time &actualRemainingDelay,
                            int &actualRunningIID );
    inline void assignTask( int &taskToAssign,
                            sc_time &actualRemainingDelay,
                            int &actualRunningIID );
    
    // time last task started
    sc_time startTime;

    void moveToRemainingPipelineStages(ProcessControlBlock* task);
    
    void setScheduler(const char *schedulername);
    
    
    void interuptPipeline(bool);

    void resumePipeline();

    void killAllTasks();

    void setTraceSignalReadyTasks(trace_value value);

  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
