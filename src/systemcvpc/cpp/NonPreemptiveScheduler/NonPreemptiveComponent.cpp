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

namespace SystemC_VPC{
    
/**
 *
 */
void NonPreemptiveComponent::updatePowerConsumption() {
  this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
  // Notify observers (e.g. powersum)
  this->fireNotification(this);
}

/*
 * from ComponentInterface
 */
bool NonPreemptiveComponent::hasWaitingOrRunningTasks() {
  /// FIXME: Implement this;
  assert(false);
  return false;
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
    
void NonPreemptiveComponent::addPowerGovernor(PluggableLocalPowerGovernor *gov) {
  this->addObserver(gov);
}

Trace::Tracing * NonPreemptiveComponent::getOrCreateTraceSignal(std::string name) {
  return taskTracer_->getOrCreateTraceSignal(name);
}


void NonPreemptiveComponent::fireStateChanged(const ComponentState &state) {
  this->setComponentState(state);
  this->updatePowerConsumption();
}

    
void NonPreemptiveComponent::setScheduler(const char *schedulername) {

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
        for (std::list<Task*>::iterator tasks_iter = mcgi->additional_tasks->begin();
             tasks_iter != mcgi->additional_tasks->end();
             tasks_iter++)
        {
          (*tasks_iter)->getBlockEvent().dii->notify();
          if ((*tasks_iter)->hasScheduledTask()) {
            assert(((*tasks_iter)->getScheduledTask())->canFire());
            ((*tasks_iter)->getScheduledTask())->scheduleLegacyWithCommState();
//                 assert(Director::canExecute((*tasks_iter)->getProcessId()));
//                 Director::execute((*tasks_iter)->getProcessId());
          }
          this->taskTracer_->finishDii((*tasks_iter));
          this->taskTracer_->finishLatency((*tasks_iter));
          Director::getInstance().signalLatencyEvent((*tasks_iter));
        }
        multiCastGroupInstances.remove(mcgi);
        delete (mcgi->additional_tasks);
        delete (mcgi);
        break;
      }
    }
  }
  fireStateChanged(ComponentState::IDLE);
  this->taskTracer_->finishDii(runningTask);

  DBG_OUT(
      this->getName() << " resign Task: " << runningTask->getName() << " @ " << sc_core::sc_time_stamp().to_default_time_units() << std::endl);

  runningTask->getBlockEvent().dii->notify();

  bool hasScheduledTask = runningTask->hasScheduledTask();
  ProcessId pid = runningTask->getProcessId();

  moveToRemainingPipelineStages(runningTask);

  Task &task = Director::getInstance().taskPool->getPrototype(pid);
  TaskInterface * scheduledTask;

  DBG_OUT("remove Task " << runningTask->getName() << std::endl);
  if (hasScheduledTask) {
    // reflect comm_state -> execute _communicate transition
    // FIXME: redesign
    scheduledTask = task.getScheduledTask();
    DBG_OUT(
        " scheduledTask: " << runningTask->getName() << " " << scheduledTask->canFire() << std::endl);

    assert(scheduledTask->canFire());
    scheduledTask->scheduleLegacyWithCommState();

//          DBG_OUT(" scheduledTask: " << runningTask->getName()
//                  << " " << Director::canExecute(pid) << std::endl);
//          assert(Director::canExecute(pid));
//          Director::execute(pid);
  }

  //TODO: PIPELINING
  runningTask = NULL;
}

/**
 *
 */
NonPreemptiveComponent::NonPreemptiveComponent(
    Config::Component::Ptr component, Director *director)
  : AbstractComponent(component)
  , runningTask(NULL)
  , blockMutex(0)
  , releasePhase(false)
{
  SC_METHOD(schedule_method);
  sensitive << notify_scheduler_thread;
  dont_initialize();

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

/**
 *
 */
void NonPreemptiveComponent::schedule_method() {
  DBG_OUT("NonPreemptiveComponent::schedule_method (" << this->getName()
      << ") triggered @" << sc_core::sc_time_stamp() << std::endl);

  //default trigger
  next_trigger(notify_scheduler_thread);

  if (runningTask == NULL) {
    releasePhase = true; // prevent from re-notifying schedule_method
    //in case of read delay (no ready task) continue releasing other actors
    while(releaseActor()/* && readyTasks.empty()*/) {}
    releasePhase = false;

    if (hasReadyTask()) {
      runningTask = scheduleTask();

      this->taskTracer_->assign(runningTask);

      next_trigger(runningTask->getRemainingDelay());

    }

    //TODO: use this as "if case" to avoid recursion!
  } else {
    assert(startTime+runningTask->getRemainingDelay() <= sc_core::sc_time_stamp());
    removeTask();
    schedule_method(); //recursion will release/schedule next task
  }
}

/**
 *
 */
void NonPreemptiveComponent::compute(Task* actualTask) {
  if(multiCastGroups.size() != 0 && multiCastGroups.find(actualTask->getProcessId()) != multiCastGroups.end()){
      //MCG vorhanden und Task auch als MultiCast zu behandeln
      MultiCastGroupInstance* instance = getMultiCastGroupInstance(actualTask);

      if(instance->task != actualTask){
        //instance already running...
        if(instance->task->getBlockEvent().latency->getDropped()){
            //handling of buffer overflow
            actualTask->getBlockEvent().latency->setDropped(true);
        }else{
            ProcessId pid = actualTask->getProcessId();
            ProcessControlBlockPtr pcb = this->getPCB(pid);
                  actualTask->setPCB(pcb);
          this->taskTracer_->release(actualTask);
        }
          return;
      }
  }

  ProcessId pid = actualTask->getProcessId();
  ProcessControlBlockPtr pcb = this->getPCB(pid);
  actualTask->setPCB(pcb);
  actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

  DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
      << " ) at time: " << sc_core::sc_time_stamp()
      << " mode: " << this->getPowerMode()->getName()
      << " schedTask: " << actualTask->getScheduledTask()
      << std::endl);

  // reset the execution delay
  actualTask->initDelays();
  //    DBG_OUT("Using " << actualTask->getRemainingDelay()
  //         << " as delay for function " << actualTask->getFunctionIds()() << "!"
  //         << std::endl);
  //    DBG_OUT("And " << actualTask->getLatency() << " as latency for function "
  //         << actualTask->getFunctionIds()() << "!" << std::endl);

  //store added task
  this->addTask(actualTask);
  this->taskTracer_->release(actualTask);

  //awake scheduler thread
  if (runningTask == NULL && !releasePhase) {
    DBG_OUT("NonPreemptiveComponent::compute (" << this->getName()
        << ") notify @" << sc_core::sc_time_stamp() << std::endl);

    notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
    //blockCompute.notify();
  }
}

/**
 *
 */
void NonPreemptiveComponent::requestBlockingCompute(
    Task* task, Coupling::VPCEvent::Ptr blocker)
{
  task->setExec(false);
  task->setBlockingCompute(blocker);
  this->compute(task);
}

/**
 *
 */
void NonPreemptiveComponent::execBlockingCompute(
    Task* task, Coupling::VPCEvent::Ptr blocker)
{
  task->setExec(true);
  blockCompute.notify();
}

/**
 *
 */
void NonPreemptiveComponent::abortBlockingCompute(
    Task* task, Coupling::VPCEvent::Ptr blocker)
{
  task->resetBlockingCompute();
  blockCompute.notify();
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
        this->taskTracer_->finishLatency(front.task);
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
void NonPreemptiveComponent::moveToRemainingPipelineStages(
    Task* task)
{
  sc_core::sc_time now = sc_core::sc_time_stamp();
  sc_core::sc_time restOfLatency = task->getLatency() - task->getDelay();
  sc_core::sc_time end = now + restOfLatency;
  if (end <= now) {
    //early exit if (Latency-DII) <= 0
    //std::cerr << "Early exit: " << task->getName() << std::endl;
    this->taskTracer_->finishLatency(task);
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

} // namespace SystemC_VPC
