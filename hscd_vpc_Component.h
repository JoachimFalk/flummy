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

#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_SchedulerProxy.h>

#include <vector.h>
#include <map.h>
#include <deque.h>

#include <smoc_event.hpp>

namespace SystemC_VPC{
  //  class SchedulerProxy;
  class Scheduler;


  /**
   * \brief The interface definition to a Virtual-Processing-Component (VPC).
   * 
   * An application using this Framework should call the AbstractComponent::compute(const char *, const char *, sc_event) Funktion.
   */
  class AbstractComponent{
  public:

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute( const char *name, const char *funcname, smoc_event *end=NULL)=0;

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute( const char *name, smoc_event *end=NULL)=0;
    //    virtual void compute(int iprocess)=0;
    virtual ~AbstractComponent(){};

    /**
     * \brief Used to create the Tracefiles.
     *
     * To create a vcd-trace-file in SystemC all the signals to 
     * trace have to be in a "global" scope. The signals have to 
     * be created in elaboration phase (before first sc_start).
     */
    virtual void informAboutMapping(string module)=0;
  };

  /**
   * \brief An implementation of AbstractComponent.
   * 
   * This is a "Virtual Component" used to simulate execution time and scheduling (also preemptive scheduling).
   * An event based communication to a Scheduler is realised, using the SchedulerProxy as counterpart.
   */
  class Component : public AbstractComponent{
  public:

    /**
     * \brief A map of tasks that newly called compute(const char *, const char *, smoc_event).
     *
     * If a task calls compute he will noted down in a map. This funktion provides
     * access to this map.
     */
    map<int,p_struct*> &getNewTasks();

    /**
     * \brief A vector of commandos, so the Scheduler can descide what to do.
     *
     * If a task calls compute, the command "ready" is generated. If the whole 
     * delay-time is delayed the command "block" is generated.
     */
    vector<action_struct> &getNewCommands();
    
    /**
     * \brief An implementation of AbstractComponent::compute(const char *, const char *, smoc_event).
     */
    virtual void compute( const char *name, const char *funcname=NULL, smoc_event *end=NULL);
    
    /**
     * \brief An implementation of AbstractComponent::compute(const char *, smoc_event).
     */
    virtual void compute( const char *name, smoc_event *end=NULL);
    //  virtual void compute(int process, smoc_event *end=NULL);
    Component(){}
    /**
     * \brief Initialize a Component with a Scheduler.
     */
    Component(const char *name,const char *schedulername);
    virtual ~Component();

    /**
     * \brief Used to create the Tracefiles.
     *
     * To create a vcd-trace-file in SystemC all the signals to 
     * trace have to be in a "global" scope. The signals have to 
     * be created in elaboration phase (before first sc_start).
     */
    virtual void informAboutMapping(string module);

  protected:
    virtual void compute(p_struct *actualTask);
    char componentName [VPC_MAX_STRING_LENGTH];
    sc_trace_file *traceFile;
    map<string,sc_signal<trace_value>*> trace_map_by_name;
  private:
    map<int,p_struct*>      newTasks;
    vector<action_struct>  open_commands;
    SchedulerProxy *schedulerproxy;
  };
  class FallbackComponent : public AbstractComponent{
  public:

    /**
     * \brief An implementation of AbstractComponent::compute(const char *, const char *, smoc_event).
     *
     * Privides backward compatibility! It does nothing -> No schedling! No delaying!
     */
    virtual void compute( const char *name, const char *funcname=NULL, smoc_event *end=NULL){
#ifdef VPC_DEBUG
      cout << flush;
      cerr << RED("FallBack::compute( ")<<WHITE(name)<<RED(" , ")<<WHITE(funcname)<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif
      if(NULL!=end) smoc_notify(*end);
    }

    /**
     * \brief An implementation of AbstractComponent::compute(const char *, smoc_event).
     *
     * Privides backward compatibility! It does nothing -> No schedling! No delaying!
     */
    virtual void compute( const char *name, smoc_event *end=NULL){
#ifdef VPC_DEBUG
      cout << flush;
      cerr << RED("FallBack::compute( ")<<WHITE(name)<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif
      if(NULL!=end) smoc_notify(*end);
    }

    /**
     * No VCD tracing in FallbackComponent needed.
     */
    virtual void informAboutMapping(string module){};

    /**
     * \brief A backward compatible implementation of AbstractComponent.
     */
    FallbackComponent(const char *name,const char *schedulername){}
    virtual ~FallbackComponent(){}
  private:
  };

  class ThreadedComponent : public Component, public SchedulerProxy/*, public sc_module*/{
    // SC_HAS_PROCESS(ThreadedComponent);
  protected:
    virtual void compute(p_struct *actualTask);
    virtual void schedule_thread(); 
  private:
    deque<p_struct*>      newTasks;
    //    map<int,action_struct> *open_commands;
    map<int,p_struct*> readyTasks,runningTasks;

    sc_event notify_scheduler_thread;
    deque<smoc_event*> events;

    inline void resignTask(int &taskToResign, sc_time &actualRemainingDelay,int &actualRunningPID);
    inline void ThreadedComponent::assignTask(int &taskToAssign, sc_time &actualRemainingDelay,int &actualRunningPID) ;
  public:


    /**
     * \brief An implementation of AbstractComponent used together with passive actors and global SMoC v2 Schedulers.
     */
    ThreadedComponent(sc_module_name name,const char *schedulername):/*sc_module*/SchedulerProxy(name){
      //SC_THREAD(schedule_thread);
      strcpy(this->componentName,name);
      setScheduler(schedulername);
      /*    schedulerproxy=new SchedulerProxy(this->componentName);
	    schedulerproxy->setScheduler(schedulername);
	    schedulerproxy->registerComponent(this);
      */
#ifndef NO_VCD_TRACES
      string tracefilename=this->componentName;
      char tracefilechar[VPC_MAX_STRING_LENGTH];
      char* traceprefix= getenv("VPCTRACEFILEPREFIX");
      if(0!=traceprefix){
	tracefilename.insert(0,traceprefix);
      }
      strcpy(tracefilechar,tracefilename.c_str());
      this->traceFile =sc_create_vcd_trace_file (tracefilechar);
      ((vcd_trace_file*)this->traceFile)->sc_set_vcd_time_unit(-9);
#endif //NO_VCD_TRACES
      
    }
    virtual ~ThreadedComponent(){}
    
  };
}
#endif
