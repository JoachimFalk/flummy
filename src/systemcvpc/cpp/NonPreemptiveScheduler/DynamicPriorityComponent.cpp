/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include "DynamicPriorityComponent.hpp"

namespace SystemC_VPC {

void DynamicPriorityComponent::notifyActivation(ScheduledTask * scheduledTask, bool active)
{
  if (active && (this->runningTask == NULL)) {
    this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
  }
}

DynamicPriorityComponent::DynamicPriorityComponent(
    Config::Component::Ptr  component, Director *director)
  : NonPreemptiveComponent(component, director)
  , mustYield_(false)
  , lastTask_(nullptr)
  , releasedTask_(nullptr)
  , debugOut(component->hasDebugFile()
      ? new Diagnostics::PrintDebug(component->getDebugFileName())
      : nullptr)
{
  buildInitialPriorityList(component);
}

DynamicPriorityComponent::~DynamicPriorityComponent() {
  if (debugOut)
    delete debugOut;
  debugOut = nullptr;
}

void DynamicPriorityComponent::buildInitialPriorityList(
    Config::Component::Ptr component) {
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

void DynamicPriorityComponent::addTask(Task *newTask) {
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

Task * DynamicPriorityComponent::scheduleTask() {
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

bool DynamicPriorityComponent::releaseActor()
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
        this->debugDump(scheduledTask);
        scheduledTask->scheduleLegacyWithCommState();
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
      this->debugDump(this->releasedTask_);
      ((this->lastTask_)->getScheduledTask())->scheduleLegacyWithCommState();
//        Director::execute(this->lastTask_->getProcessId());
      return true;
    }
    return false;
  }
}

static
std::string getActorName(ProcessId pid)
  { return Director::getInstance().getTaskName(pid); }

void DynamicPriorityComponent::debugDump(const ScheduledTask * toBeExecuted) const
{
  if (debugOut) {
    std::stringstream canExec;

    *debugOut << "@" << sc_core::sc_time_stamp() << "\t" << "[VPC DynamicPriorityComponent: "
        << this->getName() << "] " << "priority list: (";
    for (PriorityList::const_iterator iter = this->priorities_.begin(); iter
        != this->priorities_.end(); ++iter) {
      ProcessId pid = (*iter)->getPid();
      *debugOut << getActorName(pid) << " ";

      if((*iter)->canFire()){
  //      if (Director::canExecute(pid)) {
        canExec << getActorName(pid) << " ";
      }
    }
    *debugOut << ") executable: (" << canExec.str() << ") execute: " << getActorName(
        toBeExecuted->getPid()) << std::endl;
  }
}

} //namespace SystemC_VPC
