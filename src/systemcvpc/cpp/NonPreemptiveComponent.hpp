/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef __INCLUDED_FCFSCOMPONENT_H__
#define __INCLUDED_FCFSCOMPONENT_H__
#include <systemc.h>

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ComponentInfo.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/PowerSumming.hpp>
#include <systemcvpc/PowerMode.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/vpc_config.h>
#include <systemcvpc/config/Component.hpp>

#include <vector>
#include <map>
#include <deque>
#include <queue>
#include <list>


#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_FCFSCOMPONENT
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif


namespace SystemC_VPC{

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  template<class TASKTRACER>
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
    virtual void requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void updatePowerConsumption()
    {
      this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
      // Notify observers (e.g. powersum)
      this->fireNotification(this);
    }


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

    virtual Trace::Tracing * getOrCreateTraceSignal(std::string name)
    {
      return taskTracer_.getOrCreateTraceSignal(name);
    }

  protected:

    void schedule_method();

    void remainingPipelineStages();

    void fireStateChanged(const ComponentState &state)
    {
      this->setComponentState(state);
      this->updatePowerConsumption();
    }

    Task*                  runningTask;
    sc_event notify_scheduler_thread;

    // time last task started
    sc_time startTime;

    TASKTRACER taskTracer_;
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
    
    void setScheduler(const char *schedulername) {};

    void removeTask(){
      if(multiCastGroups.size() != 0 && multiCastGroups.find(runningTask->getProcessId()) != multiCastGroups.end()){
         for(std::list<MultiCastGroupInstance*>::iterator list_iter = multiCastGroupInstances.begin();
                     list_iter != multiCastGroupInstances.end(); list_iter++)
         {
           MultiCastGroupInstance* mcgi = *list_iter;
           if(mcgi->task == runningTask){
             for(std::list<Task*>::iterator tasks_iter = mcgi->additional_tasks->begin();
               tasks_iter != mcgi->additional_tasks->end(); tasks_iter++){
               (*tasks_iter)->getBlockEvent().dii->notify();
               if((*tasks_iter)->hasScheduledTask()){
                 assert(Director::canExecute((*tasks_iter)->getProcessId()));
                 Director::execute((*tasks_iter)->getProcessId());
               }
               this->taskTracer_.finishDii((*tasks_iter));
               this->taskTracer_.finishLatency((*tasks_iter));
               Director::getInstance().signalLatencyEvent((*tasks_iter));
             }
             multiCastGroupInstances.remove(mcgi);
             delete(mcgi->additional_tasks);
             delete(mcgi);
             break;
           }
         }
        }
        fireStateChanged(ComponentState::IDLE);
        this->taskTracer_.finishDii(runningTask);
        this->taskTracer_.finishLatency(runningTask);

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

  template<class TASKTRACER>
  class FcfsComponent : public NonPreemptiveComponent<TASKTRACER> {
  public:
    FcfsComponent(Config::Component::Ptr component, Director *director =
      &Director::getInstance()) :
      NonPreemptiveComponent<TASKTRACER>(component, director)
    {
    }

    virtual ~FcfsComponent() {}

    void addTask(Task *newTask)
    {
      DBG_OUT(this->getName() << " add Task: " << newTask->getName()
              << " @ " << sc_time_stamp() << std::endl);
      readyTasks.push_back(newTask);
    }


    Task * scheduleTask();

    virtual void notifyActivation(ScheduledTask * scheduledTask,
        bool active);

    virtual bool releaseActor();

    bool hasReadyTask(){
      return !readyTasks.empty();
    }
  protected:
    std::list<ScheduledTask *>       fcfsQueue;
    std::deque<Task*>                readyTasks;

  };

  template<class TASKTRACER>
  class TtFcfsComponent : public FcfsComponent<TASKTRACER> {
  public:
    TtFcfsComponent(Config::Component::Ptr component, Director *director =
      &Director::getInstance()) :
      FcfsComponent<TASKTRACER>(component, director)
    {
    }

    virtual void notifyActivation(ScheduledTask * scheduledTask, bool active)
    {
      DBG_OUT(this->name() << " notifyActivation " << scheduledTask
          << " " << active << std::endl);
      if (active) {
        if (scheduledTask->getNextReleaseTime() > sc_time_stamp()) {
          ttReleaseQueue.push(
              TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
        } else {
          this->fcfsQueue.push_back(scheduledTask);
        }
        if (this->runningTask == NULL) {
          this->notify_scheduler_thread.notify(SC_ZERO_TIME);
        }
      }
    }

    virtual bool releaseActor()
    {
      //move active TT actors to fcfsQueue
      while(!ttReleaseQueue.empty()
          && ttReleaseQueue.top().time<=sc_time_stamp()){
        this->fcfsQueue.push_back(ttReleaseQueue.top().node);
        ttReleaseQueue.pop();
      }
      bool released = FcfsComponent<TASKTRACER>::releaseActor();
      if(!ttReleaseQueue.empty() && !released){
        sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
        this->notify_scheduler_thread.notify(delta);
      }
      return released;
    }
  private:
    TT::TimedQueue ttReleaseQueue;
  };

  template<class TASKTRACER>
  class PriorityComponent : public NonPreemptiveComponent<TASKTRACER> {
  public:
    PriorityComponent(Config::Component::Ptr component, Director *director  =
        &Director::getInstance()) :
      NonPreemptiveComponent<TASKTRACER>(component, director), fcfsOrder(0)
    {
    }

    virtual ~PriorityComponent() {}

    void addTask(Task *newTask)
    {
      DBG_OUT(this->getName() << " add Task: " << newTask->getName()
              << " @ " << sc_time_stamp() << std::endl);
      p_queue_entry entry(fcfsOrder++, newTask);
      readyQueue.push(entry);
    }

    Task * scheduleTask();

    void notifyActivation(ScheduledTask * scheduledTask,
        bool active);

    bool releaseActor();

    bool hasReadyTask(){
      return !readyQueue.empty();
    }
  protected:
    size_t fcfsOrder;
    std::priority_queue<PriorityFcfsElement<ScheduledTask *> >    releaseQueue;
    std::priority_queue<p_queue_entry>                            readyQueue;

    int getPriority(const ScheduledTask * scheduledTask) {
      ProcessId pid = scheduledTask->getPid();
      ProcessControlBlockPtr pcb = this->getPCB(pid);
      return pcb->getPriority();
    }

  };

  template<class TASKTRACER>
  class TtPriorityComponent : public PriorityComponent<TASKTRACER> {
  public:
    TtPriorityComponent(Config::Component::Ptr component, Director *director =
        &Director::getInstance()) :
      PriorityComponent<TASKTRACER>(component, director)
    {
    }

    virtual void notifyActivation(ScheduledTask * scheduledTask, bool active)
    {
      DBG_OUT(this->name() << " notifyActivation " << scheduledTask
          << " " << active << std::endl);
      if (active) {
        if (scheduledTask->getNextReleaseTime() > sc_time_stamp()) {
          ttReleaseQueue.push(
              TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
        } else {
          int priority = this->getPriority(scheduledTask);
          DBG_OUT("  priority is: "<< priority << std::endl);
          this->releaseQueue.push(QueueElem(priority, this->fcfsOrder++,
              scheduledTask));
        }
        if (this->runningTask == NULL) {
          this->notify_scheduler_thread.notify(SC_ZERO_TIME);
        }
      }
    }

    virtual bool releaseActor()
    {
      //move active TT actors to fcfsQueue
      while(!ttReleaseQueue.empty()
          && ttReleaseQueue.top().time<=sc_time_stamp()){
        ScheduledTask * scheduledTask = ttReleaseQueue.top().node;
        int priority = this->getPriority(scheduledTask);
        this->releaseQueue.push(QueueElem(priority, this->fcfsOrder++,
            scheduledTask));
        ttReleaseQueue.pop();
      }
      bool released = PriorityComponent<TASKTRACER>::releaseActor();
      if(!ttReleaseQueue.empty() && !released){
        sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
        this->notify_scheduler_thread.notify(delta);
      }
      return released;
    }

  private:
    TT::TimedQueue ttReleaseQueue;
  };
/**
 *
 */
template<class TASKTRACER>
NonPreemptiveComponent<TASKTRACER>::NonPreemptiveComponent(
    Config::Component::Ptr component, Director *director) :
  AbstractComponent(component), runningTask(NULL), taskTracer_(component),
    blockMutex(0), releasePhase(false)
{
  SC_METHOD(schedule_method);
  sensitive << notify_scheduler_thread;
  dont_initialize();

  //SC_THREAD(remainingPipelineStages);

  this->setPowerMode(this->translatePowerMode("SLOW"));

  this->midPowerGov = new InternalLoadHysteresisGovernor(sc_time(12.5, SC_MS),
      sc_time(12.1, SC_MS), sc_time(4.0, SC_MS));
  this->midPowerGov->setGlobalGovernor(director->topPowerGov);

  //if(powerTables.find(getPowerMode()) == powerTables.end()){
  //  powerTables[getPowerMode()] = PowerTable();
  // }

  //PowerTable &powerTable=powerTables[getPowerMode()];
  //powerTable[ComponentState::IDLE]    = 0.0;
  //powerTable[ComponentState::RUNNING] = 1.0;

#ifndef NO_POWER_SUM
  std::string powerSumFileName(this->getName());
  powerSumFileName += ".dat";

  powerSumStream = new std::ofstream(powerSumFileName.c_str());
  powerSumming = new PowerSumming(*powerSumStream);
  this->addObserver(powerSumming);
#endif // NO_POWER_SUM
  fireStateChanged(ComponentState::IDLE);
}

/**
 *
 */
template<class TASKTRACER>
void NonPreemptiveComponent<TASKTRACER>::schedule_method()
{
  DBG_OUT("NonPreemptiveComponent::schedule_method (" << this->getName()
      << ") triggered @" << sc_time_stamp() << endl);

  //default trigger
  next_trigger(notify_scheduler_thread);

  if (runningTask == NULL) {
    releasePhase = true; // prevent from re-notifying schedule_method
    //in case of read delay (no ready task) continue releasing other actors
    while(releaseActor() && readyTasks.empty()) {}
    releasePhase = false;

    if (hasReadyTask()) {
      runningTask = scheduleTask();
      this->taskTracer_.assign(runningTask);

      next_trigger(runningTask->getRemainingDelay());

    }

    //TODO: use this as "if case" to avoid recursion!
  } else {
    assert(startTime+runningTask->getRemainingDelay() <= sc_time_stamp());
    removeTask();
    schedule_method(); //recursion will release/schedule next task
  }
}

/**
 *
 */
template<class TASKTRACER>
void NonPreemptiveComponent<TASKTRACER>::compute(Task* actualTask)
{
  if(multiCastGroups.size() != 0 && multiCastGroups.find(actualTask->getProcessId()) != multiCastGroups.end()){
      //MCG vorhanden und Task auch als MultiCast zu behandeln
      MultiCastGroupInstance* instance = getMultiCastGroupInstance(actualTask);

      if(instance->task != actualTask){
        //instance already running...
        if(instance->task->getBlockEvent().latency->getDropped()){
            //handling of buffer overflow
            actualTask->getBlockEvent().latency->setDropped(true);
        }else{
            ProcessId pid = actualTask->getProcessId();
            ProcessControlBlockPtr pcb = this->getPCB(pid);
                  actualTask->setPCB(pcb);
          this->taskTracer_.release(actualTask);
        }
          return;
      }
  }

  ProcessId pid = actualTask->getProcessId();
  ProcessControlBlockPtr pcb = this->getPCB(pid);
  actualTask->setPCB(pcb);
  actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

  DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
      << " ) at time: " << sc_time_stamp()
      << " mode: " << this->getPowerMode()->getName()
      << " schedTask: " << actualTask->getScheduledTask()
      << std::endl);

  // reset the execution delay
  actualTask->initDelays();
  //    DBG_OUT("Using " << actualTask->getRemainingDelay()
  //         << " as delay for function " << actualTask->getFunctionIds()() << "!"
  //         << std::endl);
  //    DBG_OUT("And " << actualTask->getLatency() << " as latency for function "
  //         << actualTask->getFunctionIds()() << "!" << std::endl);

  //store added task
  this->addTask(actualTask);
  this->taskTracer_.release(actualTask);

  //awake scheduler thread
  if (runningTask == NULL && !releasePhase) {
    DBG_OUT("NonPreemptiveComponent::compute (" << this->getName()
        << ") notify @" << sc_time_stamp() << endl);

    notify_scheduler_thread.notify(SC_ZERO_TIME);
    //blockCompute.notify();
  }
}

/**
 *
 */
template<class TASKTRACER>
void NonPreemptiveComponent<TASKTRACER>::requestBlockingCompute(Task* task,
    Coupling::VPCEvent::Ptr blocker)
{
  task->setExec(false);
  task->setBlockingCompute(blocker);
  this->compute(task);
}

/**
 *
 */
template<class TASKTRACER>
void NonPreemptiveComponent<TASKTRACER>::execBlockingCompute(Task* task,
    Coupling::VPCEvent::Ptr blocker)
{
  task->setExec(true);
  blockCompute.notify();
}

/**
 *
 */
template<class TASKTRACER>
void NonPreemptiveComponent<TASKTRACER>::abortBlockingCompute(Task* task,
    Coupling::VPCEvent::Ptr blocker)
{
  task->resetBlockingCompute();
  blockCompute.notify();
}

/**
 *
 */
template<class TASKTRACER>
void NonPreemptiveComponent<TASKTRACER>::remainingPipelineStages()
{
  std::cerr << "test";
  while (1) {
    if (pqueue.size() == 0) {
      wait( remainingPipelineStages_WakeUp);
    } else {
      timePcbPair front = pqueue.top();

      //cerr << "Pop from list: " << front.time << " : "
      //<< front.pcb->getBlockEvent().latency << endl;
      sc_time waitFor = front.time - sc_time_stamp();
      assert(front.time >= sc_time_stamp());
      //cerr << "Pipeline> Wait till " << front.time
      //<< " (" << waitFor << ") at: " << sc_time_stamp() << endl;
      wait(waitFor, remainingPipelineStages_WakeUp);

      sc_time rest = front.time - sc_time_stamp();
      assert(rest >= SC_ZERO_TIME);
      if (rest > SC_ZERO_TIME) {
        //cerr << "------------------------------" << endl;
      } else {
        assert(rest == SC_ZERO_TIME);
        //cerr << "Ready! releasing task (" <<  front.time <<") at: "
        //<< sc_time_stamp() << endl;

        // Latency over -> remove Task
        Director::getInstance().signalLatencyEvent(front.task);

        //wait(SC_ZERO_TIME);
        pqueue.pop();
      }
    }

  }
}

/**
 *
 */
template<class TASKTRACER>
void NonPreemptiveComponent<TASKTRACER>::moveToRemainingPipelineStages(
    Task* task)
{
  sc_time now = sc_time_stamp();
  sc_time restOfLatency = task->getLatency() - task->getDelay();
  sc_time end = now + restOfLatency;
  if (end <= now) {
    //early exit if (Latency-DII) <= 0
    //std::cerr << "Early exit: " << task->getName() << std::endl;
    Director::getInstance().signalLatencyEvent(task);
    return;
  }
  timePcbPair pair;
  pair.time = end;
  pair.task = task;
  //std::cerr << "Rest of pipeline added: " << task->getName()
  //<< " (EndTime: " << pair.time << ") " << std::endl;
  pqueue.push(pair);
  remainingPipelineStages_WakeUp.notify();
}

template<class TASKTRACER>
void FcfsComponent<TASKTRACER>::notifyActivation(ScheduledTask * scheduledTask,
    bool active)
{
  DBG_OUT(this->name() << " notifyActivation " << scheduledTask
      << " " << active << std::endl);
  if (active) {
    fcfsQueue.push_back(scheduledTask);
    if (this->runningTask == NULL) {
      this->notify_scheduler_thread.notify(SC_ZERO_TIME);
    }
  }
}

template<class TASKTRACER>
bool FcfsComponent<TASKTRACER>::releaseActor()
{
  while (!fcfsQueue.empty()) {
    ScheduledTask * scheduledTask = fcfsQueue.front();
    fcfsQueue.pop_front();

    bool canExec = Director::canExecute(scheduledTask);
    DBG_OUT("FCFS test task: " << scheduledTask
        << " -> " << canExec << std::endl);
    if (canExec) {
      Director::execute(scheduledTask);
      return true;
    }
  }

  return false;
}

template<class TASKTRACER>
Task * FcfsComponent<TASKTRACER>::scheduleTask()
{
  assert(!readyTasks.empty());
  Task* task = readyTasks.front();
  readyTasks.pop_front();
  this->startTime = sc_time_stamp();
  DBG_OUT(this->getName() << " schedule Task: " << task->getName()
      << " @ " << sc_time_stamp() << std::endl);

  /*
   * Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
   */
  if (task != NULL and task->isPSM() == true)
    this->fireStateChanged(ComponentState::IDLE);
  else
    this->fireStateChanged(ComponentState::RUNNING);

  if (task->isBlocking() /* && !assignedTask->isExec() */) {
    //TODO
  }
  return task;
}

template<class TASKTRACER>
void PriorityComponent<TASKTRACER>::notifyActivation(
    ScheduledTask * scheduledTask, bool active)
{
  DBG_OUT(this->name() << " notifyActivation " << scheduledTask
      << " " << active << std::endl);
  if (active) {
    int priority = getPriority(scheduledTask);
    DBG_OUT("  priority is: "<< priority << std::endl);
    releaseQueue.push(QueueElem(priority, fcfsOrder++, scheduledTask));
    if (this->runningTask == NULL) {
      this->notify_scheduler_thread.notify(SC_ZERO_TIME);
    }
  }
}

template<class TASKTRACER>
bool PriorityComponent<TASKTRACER>::releaseActor()
{
  while (!releaseQueue.empty()) {
    ScheduledTask * scheduledTask = releaseQueue.top().payload;
    releaseQueue.pop();

    bool canExec = Director::canExecute(scheduledTask);
    DBG_OUT("PS test task: " << scheduledTask
        << " -> " << canExec << std::endl);
    if (canExec) {
      Director::execute(scheduledTask);
      return true;
    }
  }

  return false;
}

template<class TASKTRACER>
Task * PriorityComponent<TASKTRACER>::scheduleTask()
{
  assert(!readyQueue.empty());
  Task* task = readyQueue.top().task;
  readyQueue.pop();
  this->startTime = sc_time_stamp();
  DBG_OUT(this->getName() << " schedule Task: " << task->getName()
      << " @ " << sc_time_stamp() << std::endl);

  /*
    * Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
    */
   if (task->isPSM() == true){
     this->fireStateChanged(ComponentState::IDLE);
   }else{
     this->fireStateChanged(ComponentState::RUNNING);
   }

  if (task->isBlocking() /* && !assignedTask->isExec() */) {
    //TODO
  }
  return task;
}

} // namespace SystemC_VPC
#endif //__INCLUDED_FCFSCOMPONENT_H__