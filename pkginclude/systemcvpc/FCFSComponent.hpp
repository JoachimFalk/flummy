/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * FCFSComponent.h
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
#include "datatypes.hpp"
#include "AbstractComponent.hpp"
#include "ComponentInfo.hpp"
#include "PowerSumming.hpp"
#include "PowerMode.hpp"
#include "Director.hpp"

#include <vector>
#include <map>
#include <deque>
#include <queue>


#include "debug_config.hpp"
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_FCFSCOMPONENT
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.hpp"
#else
  #include "debug_off.hpp"
#endif


namespace SystemC_VPC{

  class InternalLoadHysteresisGovernor;

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
     * \brief An implementation of AbstractComponent used together with
     * passive actors and global SMoC v2 Schedulers.
     */
    FCFSComponent( std::string name, Director *director );
      
    virtual ~FCFSComponent()
    {
      this->setPowerConsumption(0.0);
      this->fireNotification(this);
#ifndef NO_POWER_SUM
      this->removeObserver(powerSumming);
      delete powerSumming;
      delete powerSumStream;
#endif // NO_POWER_SUM
    }
    
    void addPowerGovernor(PluggableLocalPowerGovernor * gov){
      this->addObserver(gov);
    }

  protected:

    virtual void schedule_method(); 

    virtual void remainingPipelineStages(); 

  private:
    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair, std::vector<timePcbPair>,timeCompare> pqueue;

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

    virtual void notifyActivation(const ScheduledTask * scheduledTask,
        bool active);

    void addTask(Task *newTask){
      DBG_OUT(this->getName() << " add Task: " << newTask->getName()
              << " @ " << sc_time_stamp() << std::endl);
      newTask->traceReleaseTask();
      readyTasks.push_back(newTask);
    }

    void removeTask(){
        fireStateChanged(ComponentState::IDLE);
        runningTask->traceFinishTaskDii();
        runningTask->traceFinishTaskLatency();

        DBG_OUT(this->getName() << " resign Task: " << runningTask->getName()
                << " @ " << sc_time_stamp().to_default_time_units()
                << std::endl);
      
        runningTask->getBlockEvent().dii->notify();

        // signalLatencyEvent will release runningTask (back to TaskPool)
        bool hasScheduledTask = runningTask->hasScheduledTask();
        ProcessId pid = runningTask->getProcessId();

        Director::getInstance().signalLatencyEvent(runningTask);

        DBG_OUT("remove Task " << runningTask->getName() << std::endl);
        if (hasScheduledTask){
          // reflect comm_state -> execute _communicate transition
          // FIXME: redesign
          assert(Director::canExecute(pid));
          Director::execute(pid);
        }

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
      
      task->traceAssignTask();
      fireStateChanged(ComponentState::RUNNING);
      if(task->isBlocking() /* && !assignedTask->isExec() */) {
        //TODO
      }
      runningTask = task;
    }

    bool releaseActor(){
      for(ScheduledTasks::iterator iter = scheduledTasks.begin();
          iter != scheduledTasks.end();
          ++iter)
      {
        bool canExec = Director::canExecute(*iter);
        DBG_OUT("FCFS test task: " << *iter
            << " -> " << canExec << std::endl);
        if(canExec){
          Director::execute(*iter);
          return true;
        }
      }
      return false;
    }
  };

} 

#endif //__INCLUDED_FCFSCOMPONENT_H__
