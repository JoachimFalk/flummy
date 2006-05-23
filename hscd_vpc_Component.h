/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Component.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include <systemc.h>

#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_AbstractComponent.h"

#include <vector.h>
#include <map.h>
#include <deque.h>

namespace SystemC_VPC{

  //  class SchedulerProxy;
  class Scheduler;

  /**
   * \brief An implementation of AbstractComponent.
   * 
   *  Privides backward compatibility! It does nothing -> No schedling! No delaying!
   */

  class Component : public AbstractComponent{

    SC_HAS_PROCESS(Component);

    protected:

    //virtual void compute(ProcessControlBlock *actualTask);
    virtual void schedule_thread(); 

    private:

    sc_trace_file *traceFile;
    map<std::string,sc_signal<trace_value>*> trace_map_by_name;
    Scheduler *scheduler;
    deque<ProcessControlBlock*>      newTasks;
    //    map<int,action_struct> *open_commands;
    map<int,ProcessControlBlock*> readyTasks,runningTasks;

    sc_event notify_scheduler_thread;
    //deque<VPC_Event*> events;
    sc_signal<trace_value> schedulerTrace;

    inline void resignTask(int &taskToResign, sc_time &actualRemainingDelay,int &actualRunningPID);
    inline void Component::assignTask(int &taskToAssign, sc_time &actualRemainingDelay,int &actualRunningPID) ;

    // time last task started
    sc_time startTime;

    public:

    void setScheduler(const char *schedulername);

    protected:
    
    /**
     * \brief An implementation of AbstractComponent::compute(const char *, const char *, VPC_Event).
     */
    virtual void _compute( const char *name, const char *funcname=NULL, VPC_Event *end=NULL)
      __attribute__ ((deprecated));
    
    /**
     * \brief An implementation of AbstractComponent::compute(const char *, VPC_Event).
     */
    virtual void _compute( const char *name, VPC_Event *end=NULL)
      __attribute__ ((deprecated));

    public:
    
    /**
     * \brief A vector of commandos, so the Scheduler can descide what to do.
     *
     * If a task calls compute, the command "ready" is generated. If the whole 
     * delay-time is delayed the command "block" is generated.
     */
    vector<action_struct> &getNewCommands();

    /**
     * \brief Used to create the Tracefiles.
     *
     * To create a vcd-trace-file in SystemC all the signals to 
     * trace have to be in a "global" scope. The signals have to 
     * be created in elaboration phase (before first sc_start).
     */
    virtual void informAboutMapping(std::string module);

    /**
     * \brief An implementation of AbstractComponent used together with passive actors and global SMoC v2 Schedulers.
     */
    Component(sc_module_name name,const char *schedulername): AbstractComponent(name){
      SC_THREAD(schedule_thread);
      setScheduler(schedulername);
#ifndef NO_VCD_TRACES
      std::string tracefilename=this->basename(); 
      char tracefilechar[VPC_MAX_STRING_LENGTH];
      char* traceprefix= getenv("VPCTRACEFILEPREFIX");
      if(0!=traceprefix){
        tracefilename.insert(0,traceprefix);
      }
      strcpy(tracefilechar,tracefilename.c_str());
      this->traceFile = sc_create_vcd_trace_file (tracefilechar);
      ((vcd_trace_file*)this->traceFile)->sc_set_vcd_time_unit(-9);
#endif //NO_VCD_TRACES
#ifndef NO_VCD_TRACES
      sc_trace(this->traceFile,schedulerTrace,schedulername);
#endif //NO_VCD_TRACES      

      /**************************/
      /*  EXTENSION SECTION     */
      /**************************/
      if(!this->activ){
#ifdef VPC_DEBUG
        std::cerr << GREEN(this->basename() << "> Activating") << std::endl;
#endif //VPC_DEBUG
        this->setActiv(true);
      }
      /**************************/
      /*  END OF EXTENSION      */
      /**************************/
    }

    virtual ~Component(){
      sc_close_vcd_trace_file(this->traceFile);
    }
    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void processAndForwardParameter(char *sType,char *sValue);

    protected:

    // used to indicate preemption request
    sc_event notify_preempt;
    // used to indicate resume request
    sc_event notify_resume;

    public:

    /**
     * \brief Preempts execution of component
     * Used to preempt the current execution of a component.
     * \sa AbstractComponent::preempt
     */
    virtual void preempt(bool kill);

    /**
     * \brief Resumes preempted execution
     * Used to resume execution of preempted component.
     * \sa AbstractComponent
     */
    virtual void resume();

    /**
     * \brief An implementation of AbstractComponent::compute(ProcessControlBlock* , const char *).
     */
    virtual void compute(ProcessControlBlock* pcb);

    private:

    void killAllTasks();

    void setTraceSignalReadyTasks(trace_value value);

  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
