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

#include "PreemptiveComponent.hpp"

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Component.hpp>

#include "../PluggablePowerGovernor.hpp"
#include "../HysteresisLocalGovernor.hpp"
#include "../TaskInstanceImpl.hpp"
#include "../DebugOStream.hpp"
#include "../Director.hpp"

#include <CoSupport/sassert.h>

namespace SystemC_VPC { namespace Detail {

  /**
   * \brief An implementation of AbstractComponent.
   */
  PreemptiveComponent::PreemptiveComponent(std::string const &name, Scheduler *scheduler)
    : AbstractComponent(name)
    , scheduler(scheduler)
    , blockMutex(0)
  {
    SC_METHOD(ttReleaseQueueMethod);
    dont_initialize();
    sensitive << ttReleaseQueueEvent;

    SC_METHOD(ttReleaseQueuePSMMethod);
    dont_initialize();
    sensitive << ttReleaseQueuePSMEvent;

    SC_THREAD(scheduleThread);

    SC_THREAD(remainingPipelineStages);

#ifndef NO_POWER_SUM
    std::string powerSumFileName(this->getName());
    powerSumFileName += ".dat";

    powerSumStream = new std::ofstream(powerSumFileName.c_str());
    powerSumming   = new PowerSumming(*powerSumStream);
    this->addObserver(powerSumming);
#endif // NO_POWER_SUM
  }

  void PreemptiveComponent::notifyActivation(
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
      if (getTaskOfTaskInterface(scheduledTask)->getTaskIsPSM()) {
        // PSM tasks are always executed even if the component is in power down mode, i.e.,
        // !this->getCanExecuteTasks().
        if (delta > sc_core::SC_ZERO_TIME) {
          ttReleaseQueuePSM.push(
              TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
          ttReleaseQueuePSMEvent.notify(delta);
        } else {
          // This will trigger compute in due time.
          scheduledTask->schedule();
        }
      } else {
        if (delta > sc_core::SC_ZERO_TIME || !this->getCanExecuteTasks()) {
          ttReleaseQueue.push(
              TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
          ttReleaseQueueEvent.notify(delta);
        } else {
          // This will trigger compute in due time.
          scheduledTask->schedule();
        }
      }
    } else {
      if (activeTasks.find(scheduledTask) == activeTasks.end())
        // Nothing to do
        return;
      assert(!"Oops, Removal of task from the active list not supported!");
    }
  }

  static
  void ttReleaseQueueRelease(TT::TimedQueue &ttReleaseQueue, sc_core::sc_event &ttReleaseQueueEvent) {
    assert(!ttReleaseQueue.empty());

    sc_core::sc_time const &now = sc_core::sc_time_stamp();

    while (true) {
      assert(ttReleaseQueue.top().time <= now); // Less than or equal to now due to component power down mode.
      TaskInterface *scheduledTask = ttReleaseQueue.top().node;
      ttReleaseQueue.pop();
      // This will trigger compute in due time.
      scheduledTask->schedule();
      if (ttReleaseQueue.empty())
        break;
      sc_core::sc_time delta = ttReleaseQueue.top().time - now;
      if (delta > sc_core::SC_ZERO_TIME) {
        ttReleaseQueueEvent.notify(delta);
        break;
      }
    }
  }

  void PreemptiveComponent::ttReleaseQueueMethod() {
    if (!this->getCanExecuteTasks()) {
      this->requestCanExecute();
      return;
    }
    ttReleaseQueueRelease(this->ttReleaseQueue, this->ttReleaseQueueEvent);
  }

  void PreemptiveComponent::ttReleaseQueuePSMMethod() {
    ttReleaseQueueRelease(this->ttReleaseQueuePSM, this->ttReleaseQueuePSMEvent);
  }

  /*
   * used to reactivate a halted execution of the component
   */
  void PreemptiveComponent::reactivateExecution() {
    requestExecuteTasks=false;

    // Awake ttReleaseQueueMethod processing if queue not empty.
    if (!ttReleaseQueue.empty()) {
      sc_core::sc_time const &now = sc_core::sc_time_stamp();

      sc_core::sc_time delta = ttReleaseQueue.top().time - now;
      if (delta > sc_core::SC_ZERO_TIME)
        ttReleaseQueueEvent.notify(delta);
      else
        ttReleaseQueueEvent.notify(sc_core::SC_ZERO_TIME);
    }

    //awake scheduler thread
    scheduleEvent.notify();
    blockCompute.notify();
  }

  /**
   *
   */
  void PreemptiveComponent::compute(TaskInstanceImpl* actualTask){
    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
        << " ) at time: " << sc_core::sc_time_stamp()
        << " mode: " << this->getPowerMode()
        << " schedTask: " << actualTask->getTask()->getScheduledTask()
        << std::endl);
    DBG_OUT("dii: " << actualTask->getRemainingDelay() << std::endl);
    DBG_OUT("latency: " << actualTask->getLatency()  << std::endl);

    assert(this->getCanExecuteTasks() || actualTask->isPSM());
    // Now add task to the ready list of the scheduler
    this->addTask(actualTask);
  }

  /**
   *
   */
  void PreemptiveComponent::addTask(TaskInstanceImpl *newReadyTask) {
    // A task can call compute only one time!
    assert(readyTasks.find(newReadyTask->getInstanceId())   == readyTasks.end());
    assert(runningTasks.find(newReadyTask->getInstanceId()) == runningTasks.end());
    releaseTaskInstance(newReadyTask);
    //insert new task in ready list
    readyTasks[newReadyTask->getInstanceId()]=newReadyTask;
    scheduler->addedNewTask(newReadyTask);
    DBG_SC_OUT("PreemptiveComponent::addTask (" << newReadyTask->getName()
        << ") for " << this->getName() << " notifying scheduler" << std::endl);
    scheduleEvent.notify();
  }

  /**
   *
   */
  void PreemptiveComponent::scheduleThread() {
    // Time of last scheduling decision.
    sc_core::sc_time startTime = sc_core::SC_ZERO_TIME;

    scheduler->initialize();

    while (true) {
      sc_core::sc_time    now = sc_core::sc_time_stamp();
      sc_core::sc_time    runTime = now - startTime;
      assert(runTime >= sc_core::SC_ZERO_TIME);
      for (TaskMap::iterator niter, iter = runningTasks.begin();
           iter != runningTasks.end();
           iter = niter) {
        ranTaskInstance(iter->second);
//      iter->second->setRemainingDelay(iter->second->getRemainingDelay() - runTime);
        DBG_OUT(this->getName() << " IID: " << iter->first << "> Remaining delay for "
             << iter->second->getName() << " is " << iter->second->getRemainingDelay() << std::endl);
        assert(iter->second->getRemainingDelay() >= sc_core::SC_ZERO_TIME);
        if (iter->second->getRemainingDelay() == sc_core::SC_ZERO_TIME) {
          removeTask(iter->second);
          niter = runningTasks.erase(iter);
          TaskInterface *scheduledTask = iter->second->getTask()->getScheduledTask();
          if (scheduledTask) {
            // The scheduledTask->canFire() method call might call notifyActivation in
            // case that scheduledTask is a periodic actor. For this case, the
            // scheduledTask must not be present in activeTasks. Otherwise,
            // NonPreemptiveComponent::notifyActivation will ignored it and an
            // activation might be lost.
            sassert(activeTasks.erase(scheduledTask) == 1);
            if (scheduledTask->canFire()) {
              sassert(activeTasks.insert(scheduledTask).second);
              // This will trigger compute in due time.
              scheduledTask->schedule();
            }
          }
        } else
          ++(niter = iter);
      }

      int                 taskToResign = -1;
      int                 taskToAssign = -1;
      scheduling_decision decision =
        scheduler->schedulingDecision(
            taskToResign, taskToAssign, readyTasks, runningTasks);

      // Resign task, i.e., move taskToResign from  runningTasks to readyTasks.
      if (decision==RESIGNED || decision==PREEMPT) {
        TaskMap::iterator iter = runningTasks.find(taskToResign);
        assert(iter != runningTasks.end());
        DBG_OUT(this->getName() << " IID: " << iter->first << "> Resigning task " << iter->second->getName()
            << "; Remaining delay " << iter->second->getRemainingDelay() << std::endl);
        resignTaskInstance(iter->second);
        sassert(readyTasks.insert(*iter).second);
        runningTasks.erase(iter);
      }

      sc_core::sc_time waitTime;

      bool validWaitTime= scheduler->getSchedulerTimeSlice(
          waitTime, readyTasks, runningTasks);

      if (sc_core::sc_time *newOverhead = scheduler->schedulingOverhead()) {
        if (*newOverhead != sc_core::SC_ZERO_TIME) {
          wait(*newOverhead);
          if (validWaitTime) {
            assert(waitTime > *newOverhead);
            waitTime -= *newOverhead;
          }
        }
        delete newOverhead;
      }

      // Assign task, i.e.,  move taskToAssign from readyTasks to runningTasks.
      if (decision==ONLY_ASSIGN || decision==PREEMPT) {
        TaskMap::iterator iter = readyTasks.find(taskToAssign);
        assert(iter != readyTasks.end());
        DBG_OUT(this->getName() << " IID: " << iter->first << "> Assigning task " << iter->second->getName()
            << "; Remaining delay " << iter->second->getRemainingDelay() << std::endl);
        assignTaskInstance(iter->second);
        sassert(runningTasks.insert(*iter).second);
        readyTasks.erase(iter);
      }

      for (TaskMap::value_type t : runningTasks) {
        assert(t.second->getRemainingDelay() > sc_core::SC_ZERO_TIME);
        if (!validWaitTime) {
          waitTime = t.second->getRemainingDelay();
          validWaitTime = true;
        } else if (t.second->getRemainingDelay() < waitTime) {
          waitTime = t.second->getRemainingDelay();
        }
      }

      startTime = sc_core::sc_time_stamp();

      if (validWaitTime)
        wait(waitTime, scheduleEvent);
      else
        wait(scheduleEvent);
    }



/*




    sc_core::sc_time timeslice;
    sc_core::sc_time actualRemainingDelay;
    sc_core::sc_time overhead = sc_core::SC_ZERO_TIME;



//  std::string logName = getName();
//  logName = logName + ".buffer";
//  std::ofstream logBuffer(logName.c_str());
//  if( !logBuffer.is_open() ){
//    assert(false);
//  }

    //QUICKFIX solve thread initialization: actors are released before schedule_thread is called
    bool newTaskDuringOverhead= !newTasks.empty();

    while (true) {
    //  std::cout<<"Component " << this->getName() << "schedule_thread @ " << sc_core::sc_time_stamp() << std::endl;
      //determine the time slice for next scheduling decision and wait for
      bool hasTimeSlice= scheduler->getSchedulerTimeSlice( timeslice,
                                                           getReadyTasks(),
                                                           getRunningTasks());
      startTime = sc_core::sc_time_stamp();
      if(!newTaskDuringOverhead){
        if (getRunningTasks().empty()) {                    // no running task
          if(hasTimeSlice){
            wait( timeslice - overhead,
                  scheduleEvent );
          }else{
            if(!pendingTask && !hasWaitingOrRunningTasks()){
              if(!requestShutdown()){
                scheduleEvent.notify();
              }
            }
            wait( scheduleEvent );
          }
        }else{                                        // a task already runs
          if(hasTimeSlice && (timeslice - overhead) < actualRemainingDelay){
            wait( timeslice - overhead,
                  scheduleEvent );
          }else{
            wait( actualRemainingDelay,
                  scheduleEvent );
          }
          sc_core::sc_time runTime=sc_core::sc_time_stamp()-startTime;
          assert(runTime.value()>=0);
          actualRemainingDelay-=runTime;

          assert(actualRemainingDelay.value()>=0);

          DBG_OUT("Component " << this->getName()
                    << "> actualRemainingDelay= "
                    << actualRemainingDelay.value() << " for iid="
                    << actualRunningIID << " at: "
                    << sc_core::sc_time_stamp().to_default_time_units()
                    << std::endl);

          if(actualRemainingDelay.value()==0){
            removeTask(runningTasks[actualRunningIID]);
            fireStateChanged(ComponentState::IDLE);
          }else{

            // store remainingDelay
            runningTasks[actualRunningIID]->setRemainingDelay(
              actualRemainingDelay );
          }
        }
      }else{
        newTaskDuringOverhead=false;
      }

      this->addTasks();

      if(!pendingTask && !hasWaitingOrRunningTasks())
        if(!requestShutdown()){
          scheduleEvent.notify();
        }

      int taskToResign,taskToAssign;
      scheduling_decision decision =
        scheduler->schedulingDecision(taskToResign,
                                      taskToAssign,
                                      readyTasks,
                                      runningTasks);

      //resign task
      if(decision==RESIGNED || decision==PREEMPT){
        readyTasks[taskToResign]=runningTasks[taskToResign];
        runningTasks.erase(taskToResign);
        actualRunningIID=-1;
        readyTasks[taskToResign]->setRemainingDelay(actualRemainingDelay);
        fireStateChanged(ComponentState::IDLE);
        this->taskTracer_->resign(readyTasks[taskToResign]);
      }

      {
        sc_core::sc_time *newOverhead = scheduler->schedulingOverhead();
        overhead = newOverhead ? *newOverhead : sc_core::SC_ZERO_TIME;
        delete newOverhead;
      }

      if (overhead != sc_core::SC_ZERO_TIME) {
        wait(overhead);
        // true if some task becames ready during overhead waiting
        newTaskDuringOverhead = newTasks.size()>0;
      }

      //assign task
      if(decision==ONLY_ASSIGN || decision==PREEMPT){
        runningTasks[taskToAssign]=readyTasks[taskToAssign];
        this->taskTracer_->assign(runningTasks[taskToAssign]);
        readyTasks.erase(taskToAssign);
        actualRunningIID=taskToAssign;
        DBG_OUT("IID: " << taskToAssign << "> remaining delay for "
             << runningTasks[taskToAssign]->getName());
        actualRemainingDelay
          = sc_core::sc_time(runningTasks[taskToAssign]->getRemainingDelay());
        DBG_OUT(" is " << runningTasks[taskToAssign]->getRemainingDelay()
             << std::endl);

        Task * assignedTask = runningTasks[taskToAssign];

        // Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
        if (assignedTask->isPSM() == true){
            this->fireStateChanged(ComponentState::IDLE);
        }else{
            this->fireStateChanged(ComponentState::RUNNING);
        }

        if(assignedTask->isBlocking()) {
          blockMutex++;
          if(blockMutex == 1) {
            DBG_OUT(this->getName() << " scheduled blocking task: "
                    << assignedTask->getName() << std::endl);
            assignedTask->ackBlockingCompute();
            DBG_OUT(this->getName() << " enter wait: " << std::endl);
            fireStateChanged(ComponentState::STALLED);
            this->taskTracer_->block(assignedTask);
            while(!assignedTask->isExec()){
              blockCompute.reset();
              CoSupport::SystemC::wait(blockCompute);
              this->addTasks();
            }
            DBG_OUT(this->getName() << " exit wait: " << std::endl);
            fireStateChanged(ComponentState::RUNNING);
            this->taskTracer_->assign(assignedTask);
            if(assignedTask->isBlocking()){
              DBG_OUT(this->getName() << " exec Task: "
                      << assignedTask->getName() << " @  " << sc_core::sc_time_stamp()
                      << std::endl);
              // task is still blocking: exec task
            } else {
              DBG_OUT(this->getName() << " abort Task: "
                      << assignedTask->getName() << " @  " << sc_core::sc_time_stamp()
                      << std::endl);

              //notify(*(task->blockEvent));
              scheduler->removedTask(assignedTask);
              fireStateChanged(ComponentState::IDLE);
              this->taskTracer_->finishDii(assignedTask);
              //FIXME: notify latency ??
              //assignedTask->traceFinishTaskLatency();
              runningTasks.erase(actualRunningIID);

            }
          }else{
            assert(blockMutex>1);
            scheduler->removedTask(assignedTask);
            fireStateChanged(ComponentState::IDLE);
            this->taskTracer_->finishDii(assignedTask);
            //FIXME: notify latency ??
            //assignedTask->traceFinishTaskLatency();
            runningTasks.erase(actualRunningIID);
            assignedTask->abortBlockingCompute();
          }
          blockMutex--;
        }
      }
    }
//  // FIXME: Close is never reached cause of while!
//  logBuffer.close();
 */
  }

  void PreemptiveComponent::removeTask(TaskInstanceImpl *task) {
    // all execution time simulated -> BLOCK running task.

    DBG_OUT(this->getName() << " IID: " << task->getInstanceId() << " > ");
    DBG_OUT(this->getName() << " removed Task: " << task->getName()
          << " at: " << sc_core::sc_time_stamp().to_default_time_units()
          << std::endl);

    scheduler->removedTask(task);
    finishDiiTaskInstance(task);
    moveToRemainingPipelineStages(task);
    //wait(sc_core::SC_ZERO_TIME);
  }

  /**
   *
   */
  void PreemptiveComponent::requestBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker){
    task->setExec(false);
    task->setBlockingCompute( blocker );
    this->compute( task );
  }

  /**
   *
   */
  void PreemptiveComponent::execBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker){
    task->setExec(true);
    blockCompute.notify();
  }


  /**
   *
   */
  void PreemptiveComponent::abortBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker){
    task->resetBlockingCompute();
    blockCompute.notify();
  }



  /*
   * from ComponentInterface
   */
  bool PreemptiveComponent::hasWaitingOrRunningTasks()
  {
    return !readyTasks.empty() || !runningTasks.empty();
  }

  void PreemptiveComponent::initialize(const Director* d){
    //std::cerr << "Component::initialize" << std::endl;
    if(powerAttribute->isType("")){
      //std::cerr << "disabled local power governor" << std::endl;
      return;
    }

    if(NULL == localGovernorFactory){
      localGovernorFactory = new InternalLoadHysteresisGovernorFactory();
    }

    // governor parameter
    localGovernorFactory->processAttributes(powerAttribute);

    //create local governor
    midPowerGov=localGovernorFactory->createPlugIn();
    midPowerGov->setGlobalGovernor(d->topPowerGov);
    addObserver(midPowerGov);
  }

  void PreemptiveComponent::addAttribute(Attribute::Ptr attr) {
    if (attr->isType("scheduler")) {
      for(size_t i=0; i<attr->getAttributeSize();++i) {
        Attribute::Ptr schedAttr = attr->getNextAttribute(i).second;
        // FIXME: Handle unknown attributes
        scheduler->setAttribute(schedAttr);
      }
    } else
      base_type::addAttribute(attr);
  }
  bool PreemptiveComponent::addStream(ProcessId pid){
    return scheduler->addStream(pid);
  }

  bool PreemptiveComponent::closeStream(ProcessId pid){
    return scheduler->closeStream(pid);
  }


  PreemptiveComponent::~PreemptiveComponent(){
    delete scheduler;
#ifndef NO_POWER_SUM
    this->removeObserver(powerSumming);
    delete powerSumming;
    delete powerSumStream;
#endif // NO_POWER_SUM
  }

  /**
   *
   */
  void PreemptiveComponent::moveToRemainingPipelineStages(TaskInstanceImpl* task){
    sc_core::sc_time now                 = sc_core::sc_time_stamp();
    sc_core::sc_time restOfLatency       = task->getLatency()  - task->getDelay();
    sc_core::sc_time end                 = now + restOfLatency;
    if(end <= now){
      //early exit if (Latency-DII) <= 0
      //std::cerr << "Early exit: " << task->getName() << std::endl;
      finishLatencyTaskInstance(task);
      return;
    }
    timePcbPair pair;
    pair.time = end;
    pair.task  = task;
    //std::cerr << "Rest of pipeline added: " << task->getName()
    //<< " (EndTime: " << pair.time << ") " << std::endl;
    pqueue.push(pair);
    remainingPipelineStages_WakeUp.notify();
  }

  /**
   *
   */
  void PreemptiveComponent::remainingPipelineStages(){
    while(1){
      if(pqueue.size() == 0){
        wait(remainingPipelineStages_WakeUp);
      }else{
        timePcbPair front = pqueue.top();

        //std::cerr << "Pop from list: " << front.time << " : "
        //<< front.taskImpl->getBlockEvent().latency << std::endl;
        sc_core::sc_time waitFor = front.time-sc_core::sc_time_stamp();

        assert(front.time >= sc_core::sc_time_stamp());
        //std::cerr << "Pipeline> Wait till " << front.time
        //<< " (" << waitFor << ") at: " << sc_core::sc_time_stamp() << std::endl;
        wait( waitFor, remainingPipelineStages_WakeUp );

        sc_core::sc_time rest = front.time-sc_core::sc_time_stamp();
        assert(rest >= sc_core::SC_ZERO_TIME);
        if(rest > sc_core::SC_ZERO_TIME){
          //std::cerr << "------------------------------" << std::endl;
        }else{
          assert(rest == sc_core::SC_ZERO_TIME);
          //std::cerr << "Ready! releasing task (" <<  front.time <<") at: "
          //<< sc_core::sc_time_stamp() << std::endl;

          finishLatencyTaskInstance(front.task);

          //wait(sc_core::SC_ZERO_TIME);
          pqueue.pop();
        }
      }

    }
  }

} } // namespace SystemC_VPC::Detail
