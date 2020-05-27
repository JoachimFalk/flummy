// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#include "NonPreemptiveComponent.hpp"

#include "../DebugOStream.hpp"

namespace SystemC_VPC { namespace Detail {

  /**
   *
   */
  NonPreemptiveComponent::NonPreemptiveComponent(
      std::string const &name)
    : AbstractComponent(name)
    , ttReleaseQueue("ttReleaseQueue", [] (TaskInterface *t) { t->schedule(); })
    , readyTasks(0)
    , runningTask(NULL)
  {
    SC_THREAD(scheduleThread);

    this->midPowerGov = new InternalLoadHysteresisGovernor(sc_core::sc_time(12.5, sc_core::SC_MS),
        sc_core::sc_time(12.1, sc_core::SC_MS), sc_core::sc_time(4.0, sc_core::SC_MS));
    this->midPowerGov->setGlobalGovernor(Director::getInstance().topPowerGov);
  }

  void NonPreemptiveComponent::notifyActivation(
      TaskInterface *scheduledTask,
      bool           active)
  {
    DBG_SC_OUT(this->name() << " notifyActivation " << scheduledTask->name()
        << " " << active << std::endl);
    if (active) {
      if (activeTasks.find(scheduledTask) != activeTasks.end())
        // Nothing to do
        return;
      activeTasks.insert(scheduledTask);
      sc_core::sc_time delta = scheduledTask->getNextReleaseTime() - sc_core::sc_time_stamp();
      ttReleaseQueue.add(scheduledTask, delta);
    } else {
      if (activeTasks.find(scheduledTask) == activeTasks.end())
        // Nothing to do
        return;
      assert(!"Oops, Removal of task from the active list not supported!");
    }
  }

  /**
   *
   */
  void NonPreemptiveComponent::compute(TaskInstanceImpl *actualTask) {
    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
        << " ) at time: " << sc_core::sc_time_stamp()
        << " mode: " << this->getPowerMode()
        << " schedTask: " << actualTask->getTask()->getScheduledTask()
        << std::endl);
//  DBG_OUT("Using " << actualTask->getRemainingDelay()
//    << " as delay for function " << actualTask->getFunctionIds()() << "!"
//    << std::endl);
//  DBG_OUT("And " << actualTask->getLatency() << " as latency for function "
//    << actualTask->getFunctionIds()() << "!" << std::endl);

    // Now add task to the ready list of the scheduler
    this->addTask(actualTask);
  }

  void NonPreemptiveComponent::addTask(TaskInstanceImpl *newReadyTask) {
    releaseTaskInstance(newReadyTask);
    this->newReadyTask(newReadyTask);
    ++readyTasks;
    //awake scheduler thread
    if (runningTask == NULL) {
      DBG_SC_OUT("NonPreemptiveComponent::addTask (" << newReadyTask->getName()
          << ") for " << this->getName() << " notifying scheduler" << std::endl);
      scheduleEvent.notify(sc_core::SC_ZERO_TIME);
    } else {
      DBG_SC_OUT("NonPreemptiveComponent::addTask (" << newReadyTask->getName()
          << ") for " << this->getName() << std::endl);
    }
  }

  /*
   * from ComponentInterface
   */
  bool NonPreemptiveComponent::hasWaitingOrRunningTasks() {
    return runningTask != nullptr || readyTasks > 0;
  }

  /**
   *
   */
  void NonPreemptiveComponent::scheduleThread() {
    while (true) {
      while (readyTasks > 0) {
        assert(runningTask == nullptr);
        runningTask = selectReadyTask();
        assert(runningTask != nullptr);
        --readyTasks;
        assignTaskInstance(runningTask);
        wait(runningTask->getDelay());
        removeTask();
        TaskInterface *scheduledTask = runningTask->getTask()->getScheduledTask();
        if (scheduledTask) {
          // The scheduledTask->canFire() method call might call notifyActivation in
          // case that scheduledTask is a periodic actor. For this case, the
          // scheduledTask must not be present in activeTasks. Otherwise,
          // NonPreemptiveComponent::notifyActivation will ignored it and an
          // activation might be lost.
          sassert(activeTasks.erase(scheduledTask) == 1);
          if (scheduledTask->canFire())
            notifyActivation(scheduledTask, true);
        }
        runningTask = NULL;
      }
      assert(runningTask == nullptr);
      wait(scheduleEvent);
      assert(readyTasks > 0);
      DBG_SC_OUT("NonPreemptiveComponent::scheduleThread for " << this->getName() << " triggered" << std::endl);
    }
  }

  void NonPreemptiveComponent::removeTask(){
    DBG_OUT(
        this->getName() << " resign Task: " << runningTask->getName() << " @ " << sc_core::sc_time_stamp() << std::endl);
    finishDiiTaskInstance(runningTask);
  }

/**
 *
 */
void NonPreemptiveComponent::requestBlockingCompute(
    TaskInstanceImpl* task, VPCEvent::Ptr blocker)
{
  task->setExec(false);
  task->setBlockingCompute(blocker);
  this->compute(task);
}

/**
 *
 */
void NonPreemptiveComponent::execBlockingCompute(
    TaskInstanceImpl* task, VPCEvent::Ptr blocker)
{
  task->setExec(true);
  blockCompute.notify();
}

/**
 *
 */
void NonPreemptiveComponent::abortBlockingCompute(
    TaskInstanceImpl* task, VPCEvent::Ptr blocker)
{
  task->resetBlockingCompute();
  blockCompute.notify();
}

NonPreemptiveComponent::~NonPreemptiveComponent() {
  //this->setPowerConsumption(0.0);
  //this->fireNotification(this);
}

} } // namespace SystemC_VPC::Detail
