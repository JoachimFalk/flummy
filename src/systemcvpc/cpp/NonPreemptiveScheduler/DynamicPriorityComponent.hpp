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

#ifndef DYNAMICPRIORITYCOMPONENT_HPP_
#define DYNAMICPRIORITYCOMPONENT_HPP_

#include <NonPreemptiveScheduler/NonPreemptiveComponent.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/Task.hpp>

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/VpcApi.hpp>

#include "diagnostics/DebugOutput.hpp"

#include <list>

namespace SystemC_VPC
{

typedef std::list<ScheduledTask *> PriorityList;

class DynamicPriorityComponent: public NonPreemptiveComponent
{
public:

  static AbstractComponent* create(Config::Component::Ptr component);

  void addTask(Task *newTask);

  Task * scheduleTask();

  void notifyActivation(ScheduledTask * scheduledTask, bool active)
  {
    if (active && (this->runningTask == NULL)) {
      this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
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
    NonPreemptiveComponent(component, director), priorities_(),
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

template<class DEBUG_OUT>
class DynamicPriorityComponentImpl: public DynamicPriorityComponent
{
public:
  DynamicPriorityComponentImpl(Config::Component::Ptr component,
      Director *director = &Director::getInstance()) :
    DynamicPriorityComponent(component, director),
        debug_(component->getDebugFileName())
  {
  }

  bool releaseActor()
  {
    if (this->mustYield_ || (this->lastTask_ == NULL)) {
      for (PriorityList::const_iterator iter = this->priorities_.begin(); iter
          != this->priorities_.end(); ++iter) {
        ScheduledTask * scheduledTask = *iter;
        bool canExec = scheduledTask->canFire();
//        bool canExec = Director::canExecute(scheduledTask);
        if (canExec) {
          this->mustYield_ = false;
          this->releasedTask_ = scheduledTask;
          this->debugDump(debug_, scheduledTask);
          scheduledTask->schedule();
//          Director::execute(scheduledTask);
          return true;
        }
      }
      return false;
    } else {
      bool canExec = ((this->lastTask_)->getScheduledTask())->canFire();
//      bool canExec = Director::canExecute(this->lastTask_->getProcessId());
      if (canExec) {
        this->releasedTask_ = this->lastTask_->getScheduledTask();
        this->debugDump(this->debug_, this->releasedTask_);
        ((this->lastTask_)->getScheduledTask())->schedule();
//        Director::execute(this->lastTask_->getProcessId());
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

    out << "@" << sc_core::sc_time_stamp() << "\t" << "[VPC DynamicPriorityComponent: "
        << this->getName() << "] " << "priority list: (";
    for (PriorityList::const_iterator iter = this->priorities_.begin(); iter
        != this->priorities_.end(); ++iter) {
      ProcessId pid = (*iter)->getPid();
      out << getActorName(pid) << " ";

      if((*iter)->canFire()){
//      if (Director::canExecute(pid)) {
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

AbstractComponent *DynamicPriorityComponent::create(
    Config::Component::Ptr component)
{
  if (component->hasDebugFile()) {
    return new DynamicPriorityComponentImpl<Diagnostics::PrintDebug> (component);
  } else {
    return new DynamicPriorityComponentImpl<Diagnostics::DiscardOutput> (component);
  }
}

void DynamicPriorityComponent::buildInitialPriorityList(
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

void DynamicPriorityComponent::addTask(Task *newTask)
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

Task * DynamicPriorityComponent::scheduleTask()
{
  assert(this->runningTask == NULL);
  assert(lastTask_ != NULL);
  assert(releasedTask_ != NULL);
  this->startTime = sc_core::sc_time_stamp();

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
