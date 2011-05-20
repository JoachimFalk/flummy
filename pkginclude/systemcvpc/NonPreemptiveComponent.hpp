/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef __INCLUDED_FCFSCOMPONENT_H__
#define __INCLUDED_FCFSCOMPONENT_H__
#include <systemc.h>

#include <systemcvpc/vpc_config.h>
#include "config/Component.hpp"
#include "datatypes.hpp"
#include "AbstractComponent.hpp"
#include "ComponentInfo.hpp"
#include "PowerSumming.hpp"
#include "PowerMode.hpp"
#include "Director.hpp"
#include "Task.hpp"

#include <vector>
#include <map>
#include <deque>
#include <queue>
#include <list>


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
  class NonPreemptiveComponent : public AbstractComponent{
    
    SC_HAS_PROCESS(NonPreemptiveComponent);

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
    NonPreemptiveComponent(Config::Component::Ptr component, Director *director);
      
    virtual ~NonPreemptiveComponent()
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

    void schedule_method();

    void remainingPipelineStages();

    void fireStateChanged(const ComponentState &state);

    Task*                  runningTask;
    sc_event notify_scheduler_thread;

    // time last task started
    sc_time startTime;
  private:
    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair> pqueue;

    //PowerTables powerTables;
    
    Event blockCompute;
    size_t   blockMutex;
#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    InternalLoadHysteresisGovernor *midPowerGov;

    bool releasePhase;

    bool processPower(Attribute att);

    void moveToRemainingPipelineStages(Task* task);
    
    void setScheduler(const char *schedulername);

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
          DBG_OUT(" scheduledTask: " << runningTask->getName()
                  << " " << Director::canExecute(pid) << std::endl);
          assert(Director::canExecute(pid));
          Director::execute(pid);
        }

        //TODO: PIPELINING
        runningTask = NULL;
    }

    virtual void addTask(Task *newTask) = 0;

    virtual Task * scheduleTask() = 0;

    virtual void notifyActivation(ScheduledTask * scheduledTask,
        bool active) = 0;

    virtual bool releaseActor() = 0;

    virtual bool hasReadyTask() = 0;
  };

  typedef PriorityFcfsElement<ScheduledTask*>                    QueueElem;

  class FcfsComponent : public NonPreemptiveComponent {
  public:
    FcfsComponent(Config::Component::Ptr component, Director *director =
      &Director::getInstance()) :
      NonPreemptiveComponent(component, director)
    {
    }

    virtual ~FcfsComponent() {}

    void addTask(Task *newTask);

    Task * scheduleTask();

    void notifyActivation(ScheduledTask * scheduledTask,
        bool active);

    bool releaseActor();

    bool hasReadyTask(){
      return !readyTasks.empty();
    }
  private:
    std::list<ScheduledTask *>       fcfsQueue;
    std::deque<Task*>                readyTasks;

  };

  class PriorityComponent : public NonPreemptiveComponent {
  public:
    PriorityComponent(Config::Component::Ptr component, Director *director  =
        &Director::getInstance()) :
      NonPreemptiveComponent(component, director), fcfsOrder(0)
    {
    }

    virtual ~PriorityComponent() {}

    void addTask(Task *newTask);

    Task * scheduleTask();

    void notifyActivation(ScheduledTask * scheduledTask,
        bool active);

    bool releaseActor();

    bool hasReadyTask(){
      return !readyQueue.empty();
    }
  private:
    size_t fcfsOrder;
    std::priority_queue<PriorityFcfsElement<ScheduledTask *> >    releaseQueue;
    std::priority_queue<p_queue_entry>                            readyQueue;

    int getPriority(const ScheduledTask * scheduledTask) {
      ProcessId pid = scheduledTask->getPid();
      ProcessControlBlockPtr pcb = this->getPCB(pid);
      return pcb->getPriority();
    }

  };
} 

#endif //__INCLUDED_FCFSCOMPONENT_H__
