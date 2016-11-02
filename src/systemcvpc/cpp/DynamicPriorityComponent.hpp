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

#ifndef DYNAMICPRIORITYCOMPONENT_HPP_
#define DYNAMICPRIORITYCOMPONENT_HPP_

#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/Task.hpp>

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/VpcApi.hpp>

#include "NonPreemptiveComponent.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "tracing/TaskTracer.hpp"

#include <list>

namespace SystemC_VPC
{

typedef std::list<ScheduledTask *> PriorityList;

template<class TASKTRACER>
class DynamicPriorityComponent: public NonPreemptiveComponent<TASKTRACER>
{
public:

  static AbstractComponent* create(Config::Component::Ptr component);

  void addTask(Task *newTask);

  Task * scheduleTask();

  void notifyActivation(ScheduledTask * scheduledTask, bool active)
  {
    if (active && (this->runningTask == NULL)) {
      this->notify_scheduler_thread.notify(SC_ZERO_TIME);
    }
  }

  bool hasReadyTask()
  {
    return releasedTask_ != NULL;
  }

  virtual void setDynamicPriority(PriorityList priorityList)
  {

    priorities_ = priorityList;
  }

  virtual std::list<ScheduledTask *> getDynamicPriority()
  {
    return priorities_;
  }

  virtual void scheduleAfterTransition()
  {
    mustYield_ = true;
  }


protected:
  DynamicPriorityComponent(Config::Component::Ptr component,
      Director *director = &Director::getInstance()) :
    NonPreemptiveComponent<TASKTRACER> (component, director), priorities_(),
        mustYield_(false), lastTask_(NULL), releasedTask_(NULL)
  {
    buildInitialPriorityList(component);
  }

protected:
  PriorityList priorities_;
  bool mustYield_;
  Task * lastTask_;
  ScheduledTask * releasedTask_;

private:
  void buildInitialPriorityList(Config::Component::Ptr component);

};

template<class DEBUG_OUT, class TASKTRACER>
class DynamicPriorityComponentImpl: public DynamicPriorityComponent<TASKTRACER>
{
public:
  DynamicPriorityComponentImpl(Config::Component::Ptr component,
      Director *director = &Director::getInstance()) :
    DynamicPriorityComponent<TASKTRACER> (component, director),
        debug_(component->getDebugFileName())
  {
  }

  bool releaseActor()
  {
    if (this->mustYield_ || (this->lastTask_ == NULL)) {
      for (PriorityList::const_iterator iter = this->priorities_.begin(); iter
          != this->priorities_.end(); ++iter) {
        ScheduledTask * scheduledTask = *iter;
        bool canExec = Director::canExecute(scheduledTask);
        if (canExec) {
          this->mustYield_ = false;
          this->releasedTask_ = scheduledTask;
          this->debugDump(debug_, scheduledTask);
          Director::execute(scheduledTask);
          return true;
        }
      }
      return false;
    } else {
      bool canExec = Director::canExecute(this->lastTask_->getProcessId());
      if (canExec) {
        this->releasedTask_ = this->lastTask_->getScheduledTask();
        this->debugDump(this->debug_, this->releasedTask_);
        Director::execute(this->lastTask_->getProcessId());
        return true;
      }
      return false;
    }
  }

private:
  DEBUG_OUT debug_;

  void debugDump(std::ostream &out, const ScheduledTask * toBeExecuted) const
  {
    std::stringstream canExec;

    out << "@" << sc_time_stamp() << "\t" << "[VPC DynamicPriorityComponent: "
        << this->getName() << "] " << "priority list: (";
    for (PriorityList::const_iterator iter = this->priorities_.begin(); iter
        != this->priorities_.end(); ++iter) {
      ProcessId pid = (*iter)->getPid();
      out << getActorName(pid) << " ";

      if (Director::canExecute(pid)) {
        canExec << getActorName(pid) << " ";
      }
    }
    out << ") executable: (" << canExec.str() << ") execute: " << getActorName(
        toBeExecuted->getPid()) << std::endl;

  }

  std::string getActorName(ProcessId pid) const
  {
    return Director::getInstance().getTaskName(pid);
  }
};

template<class T>
AbstractComponent* DynamicPriorityComponent<T>::create(
    Config::Component::Ptr component)
{
  if (component->hasDebugFile()) {
    return new DynamicPriorityComponentImpl<Diagnostics::PrintDebug,
        T> (component);
  } else {
    return new DynamicPriorityComponentImpl<Diagnostics::DiscardOutput,
        T> (component);
  }
}

template<class TASKTRACER>
void DynamicPriorityComponent<TASKTRACER>::buildInitialPriorityList(
    Config::Component::Ptr component)
{
  std::priority_queue<PriorityFcfsElement<ScheduledTask*> > pQueue;
  size_t fcfsOrder = 0;

  // put every task in a priority queue
  Config::Component::MappedTasks mp = component->getMappedTasks();
  for (Config::Component::MappedTasks::iterator iter = mp.begin(); iter
      != mp.end(); ++iter) {
    ScheduledTask *actor = *iter;
    size_t priority = Config::getCachedTask(*actor)->getPriority();
    pQueue.push(
        PriorityFcfsElement<ScheduledTask*> (priority, fcfsOrder++, actor));
  }

  // pop tasks (in order of priority) from queue and build priority list
  while (!pQueue.empty()) {
    ScheduledTask *actor = pQueue.top().payload;
    priorities_.push_back(actor);
    pQueue.pop();
  }
}

template<class TASKTRACER>
void DynamicPriorityComponent<TASKTRACER>::addTask(Task *newTask)
{
  if (newTask->hasScheduledTask()) {
    assert(releasedTask_ == newTask->getScheduledTask());
    lastTask_ = newTask;
  } else {
    throw Config::ConfigException(
        std::string("DynamicPriorityComponent ") + this->getName()
            + " doesn't support messages! "
            + "Please do not include it in any Route!");
  }

}

template<class TASKTRACER>
Task * DynamicPriorityComponent<TASKTRACER>::scheduleTask()
{
  assert(this->runningTask == NULL);
  assert(lastTask_ != NULL);
  assert(releasedTask_ != NULL);
  this->startTime = sc_time_stamp();

  releasedTask_ = NULL;

  /*
    * Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
    */
   if (lastTask_->isPSM() == true){
     this->fireStateChanged(ComponentState::IDLE);
   }else{
     this->fireStateChanged(ComponentState::RUNNING);
   }

  return lastTask_;
}

} //namespace SystemC_VPC
#endif /* DYNAMICPRIORITYCOMPONENT_HPP_ */