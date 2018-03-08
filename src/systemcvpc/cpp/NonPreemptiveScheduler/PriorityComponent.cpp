/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of
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

#include "PriorityComponent.hpp"

#include "../DebugOStream.hpp"

namespace SystemC_VPC {

PriorityComponent::PriorityComponent(
    Config::Component::Ptr component, Director *director)
  : NonPreemptiveComponent(component, director)
  , fcfsOrder(0)
{}

PriorityComponent::~PriorityComponent() {}

void PriorityComponent::addTask(Task *newTask) {
  DBG_OUT(this->getName() << " add Task: " << newTask->getName()
          << " @ " << sc_core::sc_time_stamp() << std::endl);
  p_queue_entry entry(fcfsOrder++, newTask);
  readyQueue.push(entry);
}

int PriorityComponent::getPriority(const TaskInterface * scheduledTask) {
  return getTaskOfTaskInterface(scheduledTask)->getPriority();
}

/*
void PriorityComponent::notifyActivation(
    TaskInterface * scheduledTask, bool active) {
  DBG_OUT(this->name() << " notifyActivation " << scheduledTask
      << " " << active << std::endl);
  if (active) {
    int priority = getPriority(scheduledTask);
    DBG_OUT("  priority is: "<< priority << std::endl);
    releaseQueue.push(QueueElem(priority, fcfsOrder++, scheduledTask));
    if (this->runningTask == NULL) {
      this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
    }
  }
}
 */

void PriorityComponent::notifyActivation(TaskInterface * scheduledTask, bool active) {
  DBG_OUT(this->name() << " notifyActivation " << scheduledTask
      << " " << active << std::endl);
  if (active) {
    if (scheduledTask->getNextReleaseTime() > sc_core::sc_time_stamp()) {
      ttReleaseQueue.push(
          TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
    } else {
      int priority = this->getPriority(scheduledTask);
      DBG_OUT("  priority is: "<< priority << std::endl);
      this->releaseQueue.push(QueueElem(priority, this->fcfsOrder++,
          scheduledTask));
    }
    if (this->runningTask == NULL) {
      this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
    }
  }
}

bool PriorityComponent::releaseActor() {
  //move active TT actors to fcfsQueue
  while(!ttReleaseQueue.empty()
      && ttReleaseQueue.top().time<=sc_core::sc_time_stamp()){
    TaskInterface * scheduledTask = ttReleaseQueue.top().node;
    int priority = this->getPriority(scheduledTask);
    this->releaseQueue.push(QueueElem(priority, this->fcfsOrder++,
        scheduledTask));
    ttReleaseQueue.pop();
  }
  bool released = false;
  while (!releaseQueue.empty()) {
    TaskInterface * scheduledTask = releaseQueue.top().payload;
    releaseQueue.pop();

    bool canExec = scheduledTask->canFire();
//    bool canExec = Director::canExecute(scheduledTask);
    DBG_OUT("PS test task: " << scheduledTask
        << " -> " << canExec << std::endl);
    if (canExec) {
      scheduledTask->scheduleLegacyWithCommState();
//      Director::execute(scheduledTask);
      released = true;
      break;
    }
  }
  if(!ttReleaseQueue.empty() && !released){
    sc_core::sc_time delta = ttReleaseQueue.top().time-sc_core::sc_time_stamp();
    this->notify_scheduler_thread.notify(delta);
  }
  return released;
}

Task * PriorityComponent::scheduleTask() {
  assert(!readyQueue.empty());
  Task* task = readyQueue.top().task;
  readyQueue.pop();
  this->startTime = sc_core::sc_time_stamp();
  DBG_OUT(this->getName() << " schedule Task: " << task->getName()
      << " @ " << sc_core::sc_time_stamp() << std::endl);

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
