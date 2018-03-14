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

#include "RoundRobinComponent.hpp"
#include "../tracing/TracerIf.hpp"

namespace SystemC_VPC{

RoundRobinComponent::RoundRobinComponent(
    Config::Component::Ptr component, Director *director)
  : AbstractComponent(component)
  , useActivationCallback(false)
  , actualTask(NULL)
{
  /// FIXME: WTF?! SLOW hardcoded?
  this->setPowerMode(this->translatePowerMode("SLOW"));

  SC_THREAD(scheduleThread);
}

void RoundRobinComponent::setActivationCallback(bool flag) {
  if (useActivationCallback == flag)
    return;
  useActivationCallback = flag;
  if (!flag) {
    for (TaskInterface *scheduledTask: taskList)
      scheduledTask->setUseActivationCallback(flag);
  } else {
    for (std::vector<TaskInterface *>::iterator iter = taskList.begin();
         iter != taskList.end();
         ++iter) {
      // This will trigger notifyActivation with
      // either false when the actor is still not fireabke
      // or     true when the actor is fireable.
      (*iter)->setUseActivationCallback(true);
      // Then, notifyActivation will reset useActivationCallback
      // back to false.
      if (!useActivationCallback) {
        /// Oops, undo it
        (*iter)->setUseActivationCallback(false);
        while (iter != taskList.begin()) {
          --iter;
          (*iter)->setUseActivationCallback(false);
        }
        return;
      }
    }
  }
}

void RoundRobinComponent::end_of_elaboration() {
  PCBPool const &pcbPool = getPCBPool();
  for (PCBPool::const_iterator it=pcbPool.begin(); it!=pcbPool.end(); ++it) {
//  std::cout << "\t " << it->second->getName() << std::endl;
    Task &task = Director::getInstance().taskPool->getPrototype(it->first);
    task.setPCB(it->second.get());
    if (task.hasScheduledTask()) {
      TaskInterface *scheduledTask = task.getScheduledTask();
      scheduledTask->setUseActivationCallback(false);
      taskList.push_back(scheduledTask);
    }
  }
}

void RoundRobinComponent::notifyActivation(TaskInterface * scheduledTask, bool active) {
  if (active) {
    setActivationCallback(false);
    readyEvent.notify();
  }
}

void RoundRobinComponent::compute(Task *actualTask) {
  std::cout << "\t " << sc_core::sc_time_stamp() << " : task PID " << actualTask->getProcessId() << std::endl;
  ProcessId pid = actualTask->getProcessId();
  /// Note that actualTask is not the task prototype, i.e.,
  /// Director::getInstance().taskPool.getPrototype(pid),
  /// but an instance allocated with PrototypedPool<Task>::allocate().
  actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));
  actualTask->initDelays();
  if (actualTask->hasScheduledTask()) {
    assert(!useActivationCallback);
    scheduleMessageTasks();
    this->actualTask = actualTask;
    this->taskTracer_->release(actualTask);
    this->taskTracer_->assign(actualTask);
    std::cout << "compute: " <<  actualTask->getName() << "@" << sc_core::sc_time_stamp() << std::endl;
    wait(actualTask->getDelay());
    this->taskTracer_->finishDii(actualTask);
    this->taskTracer_->finishLatency(actualTask);
    /// This is need to trigger consumption of tokens by the actor.
    actualTask->getBlockEvent().dii->notify();
    /// FIXME: What about DII != latency
    Director::getInstance().signalLatencyEvent(actualTask);
  } else
    readyMsgTasks.push_back(actualTask);
  readyEvent.notify();
}

void RoundRobinComponent::check(Task *actualTask) {
  if (!useActivationCallback) {
    ProcessId pid = actualTask->getProcessId();
    actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));
    actualTask->initDelays();
    this->taskTracer_->release(actualTask);
    this->taskTracer_->assign(actualTask);
    wait(actualTask->getOverhead());//Director::getInstance().getOverhead() +
    this->taskTracer_->finishDii(actualTask);
    this->taskTracer_->finishLatency(actualTask);

    std::cout << "check: " <<  actualTask->getName() << "@" << sc_core::sc_time_stamp() << std::endl;
  }
}

/**
 *
 */
void RoundRobinComponent::requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker) {


}

/**
 *
 */
void RoundRobinComponent::execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker) {


}

/**
 *
 */
void RoundRobinComponent::abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker) {


}

/**
 *
 */
void RoundRobinComponent::updatePowerConsumption() {
}

/*
 * from ComponentInterface
 */
bool RoundRobinComponent::hasWaitingOrRunningTasks()
{
  /// FIXME: Implement this;
  assert(false);
  return false;
}

Trace::Tracing *RoundRobinComponent::getOrCreateTraceSignal(std::string name) {
  return taskTracer_->getOrCreateTraceSignal(name);
}

bool RoundRobinComponent::scheduleMessageTasks() {
  bool progress = !readyMsgTasks.empty();
  while (!readyMsgTasks.empty()) {
    Task *messageTask = readyMsgTasks.front();
    readyMsgTasks.pop_front();
    assert(!messageTask->hasScheduledTask());
    this->taskTracer_->release(messageTask);
    this->taskTracer_->assign(messageTask);
    /// This will setup the trigger for schedule_method to be called
    /// again when the task execution time is over.
    wait(messageTask->getDelay());
//        wait(0.9 * messageTask->getDelay());
    Director::getInstance().signalLatencyEvent(messageTask);
    this->taskTracer_->finishDii(messageTask);
    this->taskTracer_->finishLatency(messageTask);
    /// The scheduledTask, i.e., the SysteMoC actor, should now be in the comm state.
    /// Enable transition out of comm state by notifying the dii event.
    messageTask->getBlockEvent().dii->notify();
    /// FIXME: What about dii != latency
  }
  return progress;
}

void RoundRobinComponent::scheduleThread() {
  while (true) {
    bool progress = scheduleMessageTasks();
    for (TaskInterface *scheduledTask: taskList) {
      std::cout << "Checking " << scheduledTask->name() << "@" << sc_core::sc_time_stamp() << std::endl;
      while (scheduledTask->canFire()) {
        progress = true;
        assert(readyMsgTasks.empty());
        std::cout << "Scheduling " << scheduledTask->name() << "@" << sc_core::sc_time_stamp() << std::endl;
        assert(!this->actualTask);
        // This will invoke our compute callback and setup actualTask.
        scheduledTask->schedule();
        while (!actualTask ||
               !actualTask->hasScheduledTask() ||
               actualTask->getScheduledTask() != scheduledTask) {
          scheduleMessageTasks();
          if (!actualTask ||
              !actualTask->hasScheduledTask() ||
              actualTask->getScheduledTask() != scheduledTask)
            wait(readyEvent);
        }
        actualTask = nullptr;
        scheduleMessageTasks();
      }
    }
    if (!progress) {
      setActivationCallback(true);
      while (useActivationCallback) {
        wait(readyEvent);
        scheduleMessageTasks();
      }
    }
  }
}

RoundRobinComponent::~RoundRobinComponent() {}

} // namespace SystemC_VPC
