/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_FCFSComponent.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef __INCLUDED_FCFSCOMPONENT_H__
#define __INCLUDED_FCFSCOMPONENT_H__
#include <systemc.h>

#include <systemcvpc/vpc_config.h>
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


#include "debug_config.h"
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_FCFSCOMPONENT
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif


namespace SystemC_VPC{

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class FCFSComponent : public AbstractComponent{
    
    SC_HAS_PROCESS(FCFSComponent);

  public:

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(Task* task);


    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, RefCountEventPtr blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(Task* task, RefCountEventPtr blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, RefCountEventPtr blocker);
    
    /**
     *
     */
    virtual void updatePowerConsumption();

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
    FCFSComponent( sc_module_name name,
               Director *director )
      : AbstractComponent(name),
        runningTask(NULL),
        blockMutex(0)
    {
      SC_METHOD(schedule_method);
      sensitive << notify_scheduler_thread;
      dont_initialize();

      //SC_THREAD(remainingPipelineStages);

      this->setPowerMode(this->translatePowerMode("SLOW"));

      midPowerGov = new InternalLoadHysteresisGovernor(
        sc_time(12.5, SC_MS),
        sc_time(12.1, SC_MS),
        sc_time( 4.0, SC_MS));
      midPowerGov->setGlobalGovernor(director->topPowerGov);


      if(powerTables.find(getPowerMode()) == powerTables.end()){
        powerTables[getPowerMode()] = PowerTable();
      }

      PowerTable &powerTable=powerTables[getPowerMode()];
      powerTable[ComponentState::IDLE]    = 0.0;
      powerTable[ComponentState::RUNNING] = 1.0;

#ifndef NO_POWER_SUM
      std::string powerSumFileName(this->getName());
      powerSumFileName += ".dat";

      powerSumStream = new std::ofstream(powerSumFileName.c_str());
      powerSumming   = new PowerSumming(*powerSumStream);
      this->addObserver(powerSumming);
#endif // NO_POWER_SUM

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

      fireStateChanged(ComponentState::IDLE);
    }
      
    virtual ~FCFSComponent()
    {
      this->setPowerConsumption(0.0);
      this->fireNotification(this);
#ifndef NO_POWER_SUM
      this->removeObserver(powerSumming);
      delete powerSumming;
      delete powerSumStream;
#endif // NO_POWER_SUM
#ifndef NO_VCD_TRACES
      sc_close_vcd_trace_file(traceFile);
#endif //NO_VCD_TRACES      
    }

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void setAttribute(Attribute& fr_Attributes);
    
    void addPowerGovernor(PluggableLocalPowerGovernor * gov){
      this->addObserver(gov);
    }

  protected:

    virtual void schedule_method(); 

    virtual void remainingPipelineStages(); 

  private:
    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair, std::vector<timePcbPair>,timeCompare> pqueue;

#ifndef NO_VCD_TRACES
    sc_trace_file *traceFile;
    std::map<std::string, Tracing* > trace_map_by_name;
    sc_signal<trace_value> schedulerTrace;
#endif //NO_VCD_TRACES      

    std::deque<Task*>      readyTasks;
    Task*                  runningTask;

    PowerTables powerTables;
    
    sc_event notify_scheduler_thread;
    Event blockCompute;
    size_t   blockMutex;
#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    InternalLoadHysteresisGovernor *midPowerGov;

    bool processPower(Attribute att);

    // time last task started
    sc_time startTime;

    void moveToRemainingPipelineStages(Task* task);
    
    void setScheduler(const char *schedulername);
    
    void fireStateChanged(const ComponentState &state);

    void addTask(Task *newTask){
      DBG_OUT(this->getName() << " add Task: " << newTask->getName()
              << " @ " << sc_time_stamp() << std::endl);
#ifndef NO_VCD_TRACES
      if(newTask->getTraceSignal()!=0)
        newTask->getTraceSignal()->traceReady();
#endif //NO_VCD_TRACES
      readyTasks.push_back(newTask);
    }

    void removeTask(){
        fireStateChanged(ComponentState::IDLE);
#ifndef NO_VCD_TRACES
        if(runningTask->getTraceSignal()!=0)
          runningTask->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
        DBG_OUT(this->getName() << " resign Task: " << runningTask->getName()
                << " @ " << sc_time_stamp().to_default_time_units()
                << std::endl);
      
        runningTask->getBlockEvent().dii->notify();
        this->notifyParentController(runningTask);
        //TODO: PIPELINING
        runningTask = NULL;
    }

    void scheduleTask(){
      assert(!readyTasks.empty());
      Task* task = readyTasks.front();
      readyTasks.pop_front();
      startTime = sc_time_stamp();
      DBG_OUT(this->getName() << " schedule Task: " << task->getName()
              << " @ " << sc_time_stamp() << std::endl);
      
#ifndef NO_VCD_TRACES
      if(task->getTraceSignal()!=0)
        task->getTraceSignal()->traceRunning();
#endif //NO_VCD_TRACES
      fireStateChanged(ComponentState::RUNNING);
      if(task->isBlocking() /* && !assignedTask->isExec() */) {
        //TODO
      }
      runningTask = task;
    }
  };

} 

#endif //__INCLUDED_FCFSCOMPONENT_H__
