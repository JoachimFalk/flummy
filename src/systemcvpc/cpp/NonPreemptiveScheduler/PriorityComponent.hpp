/*
 * PriorityComponent.hpp
 *
 *  Created on: 16.05.2017
 *      Author: muellersi
 */

#ifndef PRIORITYCOMPONENT_HPP_
#define PRIORITYCOMPONENT_HPP_

#include <NonPreemptiveScheduler/NonPreemptiveComponent.hpp>

namespace SystemC_VPC{

  typedef PriorityFcfsElement<ScheduledTask*>                    QueueElem;

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

    virtual ~TtPriorityComponent() {}


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

      bool canExec = scheduledTask->canFire();
  //    bool canExec = Director::canExecute(scheduledTask);
      DBG_OUT("PS test task: " << scheduledTask
          << " -> " << canExec << std::endl);
      if (canExec) {
        scheduledTask->schedule();
  //      Director::execute(scheduledTask);
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

#endif /* PRIORITYCOMPONENT_HPP_ */
