/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of
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

#include "FcfsComponent.hpp"

namespace SystemC_VPC{

  FcfsComponent::FcfsComponent(Config::Component::Ptr component, Director *director)
    : NonPreemptiveComponent(component, director) {}

  FcfsComponent::~FcfsComponent() {}

  void FcfsComponent::addTask(Task *newTask) {
    DBG_OUT(this->getName() << " add Task: " << newTask->getName()
            << " @ " << sc_core::sc_time_stamp() << std::endl);
    readyTasks.push_back(newTask);
  }

  bool FcfsComponent::hasReadyTask() {
    return !readyTasks.empty();
  }

/*
  bool FcfsComponent::releaseActor() {
    while (!fcfsQueue.empty()) {
      ScheduledTask * scheduledTask = fcfsQueue.front();
      fcfsQueue.pop_front();

      bool canExec = scheduledTask->canFire();
//    bool canExec = Director::canExecute(scheduledTask);
      DBG_OUT("FCFS test task: " << scheduledTask
          << " -> " << canExec << std::endl);
      if (canExec) {
        scheduledTask->scheduleLegacyWithCommState();
//      Director::execute(scheduledTask);
        return true;
      }
    }

    return false;
  }
 */

  bool FcfsComponent::releaseActor() {
    bool released = false;

    //move active TT actors to fcfsQueue
    while(!ttReleaseQueue.empty() &&
          ttReleaseQueue.top().time <= sc_core::sc_time_stamp()) {
      this->fcfsQueue.push_back(ttReleaseQueue.top().node);
      ttReleaseQueue.pop();
    }
    while (!fcfsQueue.empty()) {
      ScheduledTask * scheduledTask = fcfsQueue.front();
      fcfsQueue.pop_front();

      bool canExec = scheduledTask->canFire();
//    bool canExec = Director::canExecute(scheduledTask);
      DBG_OUT("FCFS test task: " << scheduledTask
          << " -> " << canExec << std::endl);
      if (canExec) {
        scheduledTask->scheduleLegacyWithCommState();
//      Director::execute(scheduledTask);
        released = true;
        break;
      }
    }
    if(!ttReleaseQueue.empty() && !released) {
      sc_core::sc_time delta = ttReleaseQueue.top().time-sc_core::sc_time_stamp();
      this->notify_scheduler_thread.notify(delta);
    }
    return released;
  }

  Task * FcfsComponent::scheduleTask() {
    assert(!readyTasks.empty());
    Task* task = readyTasks.front();
    readyTasks.pop_front();
    this->startTime = sc_core::sc_time_stamp();
    DBG_OUT(this->getName() << " schedule Task: " << task->getName()
        << " @ " << sc_core::sc_time_stamp() << std::endl);

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

/*
  void FcfsComponent::notifyActivation(ScheduledTask * scheduledTask,
      bool active)
  {
    DBG_OUT(this->name() << " notifyActivation " << scheduledTask
        << " " << active << std::endl);
    if (active) {
      fcfsQueue.push_back(scheduledTask);
      if (this->runningTask == NULL) {
        this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
      }
    }
  }
 */

  void FcfsComponent::notifyActivation(ScheduledTask *scheduledTask, bool active) {
    DBG_OUT(this->name() << " notifyActivation " << scheduledTask
        << " " << active << std::endl);
    if (active) {
      if (scheduledTask->getNextReleaseTime() > sc_core::sc_time_stamp()) {
        ttReleaseQueue.push(
            TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
      } else {
        this->fcfsQueue.push_back(scheduledTask);
      }
      if (this->runningTask == NULL) {
        this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
      }
    }
  }

} // namespace SystemC_VPC
