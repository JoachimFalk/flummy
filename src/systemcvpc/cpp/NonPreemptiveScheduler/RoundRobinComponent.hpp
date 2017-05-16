/*
 * RoundRobinComponent.hpp
 *
 *  Created on: 12.05.2017
 *      Author: muellersi
 */

#ifndef ROUNDROBINCOMPONENT_HPP_
#define ROUNDROBINCOMPONENT_HPP_

#include <NonPreemptiveScheduler/NonPreemptiveComponent.hpp>

namespace SystemC_VPC{

  template<class TASKTRACER>
  class RoundRobinComponent : public NonPreemptiveComponent<TASKTRACER> {
  public:
    RoundRobinComponent(Config::Component::Ptr component, Director *director =
      &Director::getInstance()) :
      NonPreemptiveComponent<TASKTRACER>(component, director)
    {
    }

    virtual ~RoundRobinComponent() {}

    void addTask(Task *newTask)
    {
      DBG_OUT(this->getName() << " add Task: " << newTask->getName()
              << " @ " << sc_time_stamp() << std::endl);
      readyTasks.push_back(newTask);
    }


    Task * scheduleTask();

    virtual void notifyActivation(ScheduledTask * scheduledTask,        // == setProperty
        bool active);

    virtual bool releaseActor(); // == getSchedulerTimeSlice

    bool hasReadyTask(){
      return !readyTasks.empty();
    }
  protected:
    std::list<ScheduledTask *>       fcfsQueue;
    std::deque<Task*>                readyTasks;

  };

  template<class TASKTRACER>
  void RoundRobinComponent<TASKTRACER>::notifyActivation(ScheduledTask * scheduledTask,
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
  bool RoundRobinComponent<TASKTRACER>::releaseActor()
  {
    while (!fcfsQueue.empty()) {
      ScheduledTask * scheduledTask = fcfsQueue.front();
      fcfsQueue.pop_front();

      bool canExec = scheduledTask->canFire();
      DBG_OUT("FCFS test task: " << scheduledTask
          << " -> " << canExec << std::endl);
      if (canExec) {
        scheduledTask->schedule();
        return true;
      }
    }

    return false;
  }

  template<class TASKTRACER>
  Task * RoundRobinComponent<TASKTRACER>::scheduleTask()
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

} // namespace SystemC_VPC

#endif /* ROUNDROBINCOMPONENT_HPP_ */
