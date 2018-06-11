// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#include "NonPreemptiveComponent.hpp"

#include "../DebugOStream.hpp"

namespace SystemC_VPC {

  /**
   *
   */
  NonPreemptiveComponent::NonPreemptiveComponent(
      Config::Component::Ptr component, Director *director)
    : AbstractComponent(component)
    , readyTasks(0)
    , runningTask(NULL)
  {
    SC_METHOD(ttReleaseQueueMethod);
    dont_initialize();
    sensitive << ttReleaseQueueEvent;

    SC_THREAD(scheduleThread);

    SC_THREAD(remainingPipelineStages);

    this->setPowerMode(this->translatePowerMode("SLOW"));

    this->midPowerGov = new InternalLoadHysteresisGovernor(sc_core::sc_time(12.5, sc_core::SC_MS),
        sc_core::sc_time(12.1, sc_core::SC_MS), sc_core::sc_time(4.0, sc_core::SC_MS));
    this->midPowerGov->setGlobalGovernor(director->topPowerGov);



    //if(powerTables.find(getPowerMode()) == powerTables.end()){
    //  powerTables[getPowerMode()] = PowerTable();
    // }

    //PowerTable &powerTable=powerTables[getPowerMode()];
    //powerTable[ComponentState::IDLE]    = 0.0;
    //powerTable[ComponentState::RUNNING] = 1.0;

#ifndef NO_POWER_SUM
    std::string powerSumFileName(this->getName());
    powerSumFileName += ".dat";

    powerSumStream = new std::ofstream(powerSumFileName.c_str());
    powerSumming = new PowerSumming(*powerSumStream);
    this->addObserver(powerSumming);
#endif // NO_POWER_SUM
    fireStateChanged(ComponentState::IDLE);
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
      if (delta > sc_core::SC_ZERO_TIME) {
        ttReleaseQueue.push(
            TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
        ttReleaseQueueEvent.notify(delta);
      } else {
        // This will trigger compute(getTaskOfTaskInterface(scheduledTask)) in due time.
        scheduledTask->schedule();
//      addTask(getTaskOfTaskInterface(scheduledTask));
      }
    } else {
      if (activeTasks.find(scheduledTask) == activeTasks.end())
        // Nothing to do
        return;
      assert(!"Oops, Removal of task from the active list not supported!");
    }
  }

  void NonPreemptiveComponent::ttReleaseQueueMethod() {
    assert(!ttReleaseQueue.empty());
    while (true) {
      assert(ttReleaseQueue.top().time == sc_core::sc_time_stamp());
      TaskInterface *scheduledTask = ttReleaseQueue.top().node;
      ttReleaseQueue.pop();
      // This will trigger compute(getTaskOfTaskInterface(scheduledTask)) in due time.
      scheduledTask->schedule();
//    addTask(getTaskOfTaskInterface(scheduledTask));
      if (ttReleaseQueue.empty())
        break;
      sc_core::sc_time delta = ttReleaseQueue.top().time -
          sc_core::sc_time_stamp();
      if (delta > sc_core::SC_ZERO_TIME) {
        ttReleaseQueueEvent.notify(delta);
        break;
      }
    }
  }

  /**
   *
   */
  void NonPreemptiveComponent::compute(TaskInstance *actualTask) {
    if (multiCastGroups.size() != 0 && multiCastGroups.find(actualTask->getProcessId()) != multiCastGroups.end()) {
      //MCG vorhanden und Task auch als MultiCast zu behandeln
      MultiCastGroupInstance* instance = getMultiCastGroupInstance(actualTask);

      if (instance->task != actualTask) {
        //instance already running...
        if (instance->task->getBlockEvent().latency->getDropped()) {
          //handling of buffer overflow
          actualTask->getBlockEvent().latency->setDropped(true);
        } else {
          ProcessId pid = actualTask->getProcessId();
          actualTask->setPCB(getPCB(pid));
          releaseTask(actualTask);
        }
        return;
      }
    }

    ProcessId pid = actualTask->getProcessId();
    actualTask->setPCB(getPCB(pid));
    actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
        << " ) at time: " << sc_core::sc_time_stamp()
        << " mode: " << this->getPowerMode()->getName()
        << " schedTask: " << actualTask->getScheduledTask()
        << std::endl);

    // reset the execution delay
    actualTask->initDelays();
//  DBG_OUT("Using " << actualTask->getRemainingDelay()
//    << " as delay for function " << actualTask->getFunctionIds()() << "!"
//    << std::endl);
//  DBG_OUT("And " << actualTask->getLatency() << " as latency for function "
//    << actualTask->getFunctionIds()() << "!" << std::endl);

    // Now add task to the ready list of the scheduler
    this->addTask(actualTask);
  }

  void NonPreemptiveComponent::addTask(TaskInstance *newReadyTask) {
    releaseTask(newReadyTask);
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
        if (!runningTask->isPSM())
          fireStateChanged(ComponentState::RUNNING);
        else
          // Assuming PSM actors are assigned to the same component they model,
          // the executing state of the component should be IDLE.
          fireStateChanged(ComponentState::IDLE);
        wait(runningTask->getDelay());
        removeTask();
        TaskInterface *scheduledTask = runningTask->getScheduledTask();
        if (scheduledTask) {
          // The scheduledTask->canFire() method call might call notifyActivation in
          // case that scheduledTask is a periodic actor. For this case, the
          // scheduledTask must not be present in activeTasks. Otherwise,
          // NonPreemptiveComponent::notifyActivation will ignored it and an
          // activation might be lost.
          sassert(activeTasks.erase(scheduledTask) == 1);
          if (scheduledTask->canFire()) {
            sassert(activeTasks.insert(scheduledTask).second);
            // This will trigger compute(getTaskOfTaskInterface(scheduledTask)) in due time.
            scheduledTask->schedule();
          }
        }
        runningTask = NULL;
      }
      assert(runningTask == nullptr);
      fireStateChanged(ComponentState::IDLE);
      wait(scheduleEvent);
      assert(readyTasks > 0);
      DBG_SC_OUT("NonPreemptiveComponent::scheduleThread for " << this->getName() << " triggered" << std::endl);
    }
  }

  void NonPreemptiveComponent::removeTask(){
    if (multiCastGroups.size() != 0 &&
        multiCastGroups.find(runningTask->getProcessId())
            != multiCastGroups.end())
    {
      for (std::list<MultiCastGroupInstance*>::iterator list_iter = multiCastGroupInstances.begin();
           list_iter != multiCastGroupInstances.end();
           list_iter++)
      {
        MultiCastGroupInstance* mcgi = *list_iter;
        if (mcgi->task == runningTask) {
          for (std::list<TaskInstance*>::iterator tasks_iter = mcgi->additional_tasks->begin();
               tasks_iter != mcgi->additional_tasks->end();
               tasks_iter++)
          {
            (*tasks_iter)->getBlockEvent().dii->notify();
            finishDiiTaskInstance(*tasks_iter);
            finishLatencyTaskInstance(*tasks_iter);
            Director::getInstance().signalLatencyEvent(*tasks_iter);
          }
          multiCastGroupInstances.remove(mcgi);
          delete (mcgi->additional_tasks);
          delete (mcgi);
          break;
        }
      }
    }
    fireStateChanged(ComponentState::IDLE);
    finishDiiTaskInstance(runningTask);

    DBG_OUT(
        this->getName() << " resign Task: " << runningTask->getName() << " @ " << sc_core::sc_time_stamp().to_default_time_units() << std::endl);

    runningTask->getBlockEvent().dii->notify();

//  ProcessId pid = runningTask->getProcessId();
//  Task &task = Director::getInstance().taskPool->getPrototype(pid);
//  TaskInterface * scheduledTask;

    moveToRemainingPipelineStages(runningTask);

    DBG_OUT("remove Task " << runningTask->getName() << std::endl);
  }

  /**
   *
   */
  void NonPreemptiveComponent::moveToRemainingPipelineStages(
      TaskInstance* task)
  {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    sc_core::sc_time restOfLatency = task->getLatency() - task->getDelay();
    sc_core::sc_time end = now + restOfLatency;
    if (end <= now) {
      //early exit if (Latency-DII) <= 0
      //std::cerr << "Early exit: " << task->getName() << std::endl;
      finishLatencyTaskInstance(task);
      // signalLatencyEvent will release runningTask (back to TaskPool)
      Director::getInstance().signalLatencyEvent(task);
      return;
    }
    timePcbPair pair;
    pair.time = end;
    pair.task = task;
    //std::cerr << "Rest of pipeline added: " << task->getName()
    //<< " (EndTime: " << pair.time << ") " << std::endl;
    pqueue.push(pair);
    remainingPipelineStages_WakeUp.notify();
  }

  /**
   *
   */
  void NonPreemptiveComponent::remainingPipelineStages() {
    while (1) {
      if (pqueue.size() == 0) {
        wait( remainingPipelineStages_WakeUp);
      } else {
        timePcbPair front = pqueue.top();

        //std::cerr << "Pop from list: " << front.time << " : "
        //<< front.pcb->getBlockEvent().latency << std::endl;
        sc_core::sc_time waitFor = front.time - sc_core::sc_time_stamp();
        assert(front.time >= sc_core::sc_time_stamp());
        //std::cerr << "Pipeline> Wait till " << front.time
        //<< " (" << waitFor << ") at: " << sc_core::sc_time_stamp() << std::endl;
        wait(waitFor, remainingPipelineStages_WakeUp);

        sc_core::sc_time rest = front.time - sc_core::sc_time_stamp();
        assert(rest >= sc_core::SC_ZERO_TIME);
        if (rest > sc_core::SC_ZERO_TIME) {
          //std::cerr << "------------------------------" << std::endl;
        } else {
          assert(rest == sc_core::SC_ZERO_TIME);
          //std::cerr << "Ready! releasing task (" <<  front.time <<") at: "
          //<< sc_core::sc_time_stamp() << std::endl;

          // Latency over -> remove Task
          finishLatencyTaskInstance(front.task);
          // signalLatencyEvent will release runningTask (back to TaskPool)
          Director::getInstance().signalLatencyEvent(front.task);

          //wait(sc_core::SC_ZERO_TIME);
          pqueue.pop();
        }
      }
    }
  }

/**
 *
 */
void NonPreemptiveComponent::updatePowerConsumption() {
  this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
  // Notify observers (e.g. powersum)
  this->fireNotification(this);
}

void NonPreemptiveComponent::fireStateChanged(const ComponentState &state) {
  this->setComponentState(state);
  this->updatePowerConsumption();
}

/**
 *
 */
void NonPreemptiveComponent::requestBlockingCompute(
    TaskInstance* task, Coupling::VPCEvent::Ptr blocker)
{
  task->setExec(false);
  task->setBlockingCompute(blocker);
  this->compute(task);
}

/**
 *
 */
void NonPreemptiveComponent::execBlockingCompute(
    TaskInstance* task, Coupling::VPCEvent::Ptr blocker)
{
  task->setExec(true);
  blockCompute.notify();
}

/**
 *
 */
void NonPreemptiveComponent::abortBlockingCompute(
    TaskInstance* task, Coupling::VPCEvent::Ptr blocker)
{
  task->resetBlockingCompute();
  blockCompute.notify();
}

NonPreemptiveComponent::~NonPreemptiveComponent() {
  this->setPowerConsumption(0.0);
  this->fireNotification(this);
#ifndef NO_POWER_SUM
  this->removeObserver(powerSumming);
  delete powerSumming;
  delete powerSumStream;
#endif // NO_POWER_SUM
}

} // namespace SystemC_VPC
