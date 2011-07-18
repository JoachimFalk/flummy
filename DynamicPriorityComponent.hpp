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

#include <systemcvpc/NonPreemptiveComponent.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/Task.hpp>

#include <systemcvpc/config/Component.hpp>

#include "DebugOutput.hpp"

#include <list>

namespace SystemC_VPC
{

class DynamicPriorityComponent: public NonPreemptiveComponent
{
public:
  typedef std::list<ScheduledTask *> PriorityList;

  static DynamicPriorityComponent* create(Config::Component::Ptr component);

  void addTask(Task *newTask);

  Task * scheduleTask();

  void notifyActivation(ScheduledTask * scheduledTask, bool active);

  bool hasReadyTask();

  virtual void setDynamicPriority(PriorityList priorityList);
  virtual void scheduleAfterTransition();

protected:
  DynamicPriorityComponent(Config::Component::Ptr component,
      Director *director = &Director::getInstance());

protected:
  PriorityList priorities_;
  bool mustYield_;
  Task * lastTask_;
  ScheduledTask * releasedTask_;

private:
  void buildInitialPriorityList(Config::Component::Ptr component);

};

template<class DEBUG_OUT>
class DynamicPriorityComponentImpl : public DynamicPriorityComponent{
public:
  DynamicPriorityComponentImpl(Config::Component::Ptr component,
      Director *director = &Director::getInstance()) :
    DynamicPriorityComponent(component, director),
    debug_(component->getDebugFileName())
  {
  }

  bool releaseActor()
  {
    if (mustYield_ || (lastTask_ == NULL)) {
      for (PriorityList::const_iterator iter = priorities_.begin(); iter
          != priorities_.end(); ++iter) {
        ScheduledTask * scheduledTask = *iter;
        bool canExec = Director::canExecute(scheduledTask);
        if (canExec) {
          mustYield_ = false;
          releasedTask_ = scheduledTask;
          this->debugDump(debug_, scheduledTask);
          Director::execute(scheduledTask);
          return true;
        }
      }
      return false;
    } else {
      bool canExec = Director::canExecute(lastTask_->getProcessId());
      if (canExec) {
        releasedTask_ = lastTask_->getScheduledTask();
        this->debugDump(debug_, releasedTask_);
        Director::execute(lastTask_->getProcessId());
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
    for (PriorityList::const_iterator iter = priorities_.begin(); iter
        != priorities_.end(); ++iter) {
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

} //namespace SystemC_VPC
#endif /* DYNAMICPRIORITYCOMPONENT_HPP_ */
