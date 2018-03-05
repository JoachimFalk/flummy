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
#include "AVBScheduler.hpp"
#include "FlexRayScheduler.hpp"
#include "MostScheduler.hpp"
#include "MostSecondaryScheduler.hpp"
#include "PriorityScheduler.hpp"
#include "RateMonotonicScheduler.hpp"
#include "RoundRobinScheduler.hpp"
#include "Scheduler.hpp"
#include "StreamShaperScheduler.hpp"
#include "TDMAScheduler.hpp"
#include "TimeTriggeredCCScheduler.hpp"

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/datatypes.hpp>
#include "../PluggablePowerGovernor.hpp"
#include "../HysteresisLocalGovernor.hpp"
#include "../Task.hpp"
#include <systemcvpc/config/Component.hpp>

#include "../DebugOStream.hpp"

namespace SystemC_VPC{

  /**
   * \brief An implementation of AbstractComponent.
   */
  PreemptiveComponent::PreemptiveComponent(Config::Component::Ptr component)
    : AbstractComponent(component),
      blockMutex(0), max_used_buffer(0), max_avail_buffer(0)
  {
    SC_THREAD(schedule_thread);
    SC_THREAD(remainingPipelineStages);
    SC_METHOD(releaseActorsMethod);
    dont_initialize();
    sensitive << releaseActors;
    this->setPowerMode(this->translatePowerMode("SLOW"));
    setScheduler(component);

#ifndef NO_POWER_SUM
    std::string powerSumFileName(this->getName());
    powerSumFileName += ".dat";

    powerSumStream = new std::ofstream(powerSumFileName.c_str());
    powerSumming   = new PowerSumming(*powerSumStream);
    this->addObserver(powerSumming);
#endif // NO_POWER_SUM
  }

  /**
   *
   */
  void PreemptiveComponent::setScheduler(Config::Component::Ptr component)
  {
    Config::Scheduler schedulerType = component->getScheduler();
    switch (schedulerType) {
      case Config::Scheduler::RoundRobin:
        scheduler = new RoundRobinScheduler();
        break;
      case Config::Scheduler::StaticPriority_P:
        scheduler = new PriorityScheduler();
        break;
      case Config::Scheduler::RateMonotonic:
        scheduler = new RateMonotonicScheduler();
        break;
      case Config::Scheduler::TDMA:
        scheduler = new TDMAScheduler();
        break;
      case Config::Scheduler::FlexRay:
        scheduler = new FlexRayScheduler();
        break;
      case Config::Scheduler::AVB:
        scheduler = new AVBScheduler();
        break;
      case Config::Scheduler::TTCC:
        scheduler = new TimeTriggeredCCScheduler();
        break;
      case Config::Scheduler::MOST:
        scheduler = new MostScheduler();
        break;
      case Config::Scheduler::StreamShaper:
        scheduler = new StreamShaperScheduler();
        break;
      default:
        assert(!"Oops, I don't know this scheduler!");
    }
  }

  /**
   *
   */
  void PreemptiveComponent::compute(Task* actualTask){
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
    pendingTask = false;
    if(max_avail_buffer == 0 || (getReadyTasks().size() + newTasks.size() + tasksDuringNoExecutionPhase.size()) < max_avail_buffer){
      ProcessId pid = actualTask->getProcessId();
      ProcessControlBlockPtr pcb = this->getPCB(pid);
      actualTask->setPCB(pcb);
      actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

      DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
              << " ) at time: " << sc_core::sc_time_stamp()
              << " mode: " << this->getPowerMode()->getName()
              << std::endl);

      // reset the execution delay
      actualTask->initDelays();
      DBG_OUT("dii: " << actualTask->getRemainingDelay() << std::endl);
      DBG_OUT("latency: " << actualTask->getLatency()  << std::endl);

      //store added task
      newTasks.push_back(actualTask);

      if(this->getCanExecuteTasks() || actualTask->isPSM()){
        //awake scheduler thread
        notify_scheduler_thread.notify();
        blockCompute.notify();
      }else{
        //TODO: This has to notify the scheduler after the CanExecute is set correctly
        this->requestCanExecute();
      }
    }else{
      //std::cout<< "Message/Task " << actualTask->getName() << " dropped due to less buffer-space of component "<< getName() << " @ " << sc_core::sc_time_stamp() << std::endl;
      actualTask->getBlockEvent().latency->setDropped(true);
    }
  }



  /**
   *
   */
  void PreemptiveComponent::requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker){
    task->setExec(false);
    task->setBlockingCompute( blocker );
    this->compute( task );
  }

  /**
   *
   */
  void PreemptiveComponent::execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker){
    task->setExec(true);
    blockCompute.notify();
  }


  /**
   *
   */
  void PreemptiveComponent::abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker){
    task->resetBlockingCompute();
    blockCompute.notify();
  }


  void PreemptiveComponent::updatePowerConsumption()
  {
    this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
    // Notify observers (e.g. powersum)
    this->fireNotification(this);
  }

  Trace::Tracing *PreemptiveComponent::getOrCreateTraceSignal(std::string name)
  {
    return taskTracer_->getOrCreateTraceSignal(name);
  }

  /*
   * from ComponentInterface
   */
  bool PreemptiveComponent::hasWaitingOrRunningTasks()
  {
    //std::cout<<"hasWaitingOrRunningTasks() " << readyTasks.size() <<" " <<  runningTasks.size() << " " << tasksDuringNoExecutionPhase.size() << std::endl;
    return (getReadyTasks().size() + getRunningTasks().size() + tasksDuringNoExecutionPhase.size()) > 0;
  }

  void PreemptiveComponent::fireStateChanged(const ComponentState &state)
  {
    this->setComponentState(state);
    this->updatePowerConsumption();
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
    this->addPowerGovernor(midPowerGov);
    
  }

  bool PreemptiveComponent::setAttribute(AttributePtr attribute){
    bool isComponentAttribute = AbstractComponent::setAttribute(attribute);
    if (!isComponentAttribute){
      scheduler->setAttribute(attribute);
    }
    return true;
  }
  bool PreemptiveComponent::addStream(ProcessId pid){
    return scheduler->addStream(pid);
  }

  bool PreemptiveComponent::closeStream(ProcessId pid){
    return scheduler->closeStream(pid);
  }


  PreemptiveComponent::~PreemptiveComponent(){
    this->setPowerConsumption(0.0);
    this->fireNotification(this);
    //std::cout<<"MAX used Buffer of Component " << this->name() << " was " << max_used_buffer << std::endl;
#ifndef NO_POWER_SUM
    this->removeObserver(powerSumming);
    delete powerSumming;
    delete powerSumStream;
#endif // NO_POWER_SUM
  }

/*
 * used to reactivate a halted execution of the component
 */
  void PreemptiveComponent::reactivateExecution(){
    requestExecuteTasks=false;
    while(!tasksDuringNoExecutionPhase.empty()){
      TT::TimeNodePair pair = tasksDuringNoExecutionPhase.front();
      ttReleaseQueue.push(pair);
      tasksDuringNoExecutionPhase.pop_front();
    }

    if(!ttReleaseQueue.empty()){
      if(ttReleaseQueue.top().time<=sc_core::sc_time_stamp()){
        releaseActors.notify();
      }else{
       sc_core::sc_time delta = ttReleaseQueue.top().time-sc_core::sc_time_stamp();
       releaseActors.notify(delta);
      }
    }
    //awake scheduler thread
    notify_scheduler_thread.notify();
    blockCompute.notify();
  }

  void PreemptiveComponent::notifyActivation(TaskInterface * scheduledTask,
      bool active){
    if(active) {
      TT::TimeNodePair newTask = TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask);
      //std::cout<<"Component " << this->getName() << " notifyActivation("<<scheduledTask->getPid()<<", " << (this->getPCB(scheduledTask->getPid()))->getName() << " isPSM=" << this->getPCB(scheduledTask->getPid())->isPSM() << " @ " << newTask.time << " @ " << sc_core::sc_time_stamp() << std::endl;
      if(this->getCanExecuteTasks() || getPCBofTask(scheduledTask)->isPSM()){
        pendingTask = true;
        ttReleaseQueue.push(newTask);

        if(ttReleaseQueue.top().time<=sc_core::sc_time_stamp()){
          releaseActors.notify(sc_core::SC_ZERO_TIME);
        }else{
         sc_core::sc_time delta = ttReleaseQueue.top().time-sc_core::sc_time_stamp();
         releaseActors.notify(delta);
        }
      }else{
        tasksDuringNoExecutionPhase.push_back(newTask);
        this->requestCanExecute();
      }
    }
  }

  void PreemptiveComponent::releaseActorsMethod(){
//     if(this->getCanExecuteTasks()){ //not required, no "normal" task will be added to ttReleaseQueue
    //std::cout<<"Component " << this->getName() << " releaseActorsMethod " << ttReleaseQueue.top().node->getPid() << " @ " << sc_core::sc_time_stamp() << std::endl;
      TT::TimeNodePair tnp = ttReleaseQueue.top();
      if(tnp.time <= sc_core::sc_time_stamp()){
        ttReleaseQueue.pop();
        assert(tnp.time <= sc_core::sc_time_stamp());
        if(this->getCanExecuteTasks() || getPCBofTask(tnp.node)->isPSM()){
          if(tnp.node->canFire()){
            tnp.node->scheduleLegacyWithCommState();
//          if(Director::canExecute(tnp.node)){
//            Director::execute(tnp.node);
            pendingTask = true;
          }else{
            pendingTask = false;
            notify_scheduler_thread.notify();
          }
        }else{
          tasksDuringNoExecutionPhase.push_back(tnp);
        }
      }else{
        //std::cout<<"Spezial!" << std::endl;
      }

      if(!ttReleaseQueue.empty()){
        if(ttReleaseQueue.top().time<=sc_core::sc_time_stamp()){
          // The sc_core::SC_ZERO_TIME is needed, otherwise releaseActorsMethod won't
          // be recalled.
          releaseActors.notify(sc_core::SC_ZERO_TIME);
        }else{
          assert(ttReleaseQueue.top().time>=sc_core::sc_time_stamp());
          sc_core::sc_time delta = ttReleaseQueue.top().time-sc_core::sc_time_stamp();
          releaseActors.notify(delta);
        }
      }
  }

  /**
   *
   */
  void PreemptiveComponent::addTasks() {
    if(!this->getCanExecuteTasks()){
      std::deque<Task*>::iterator iter = newTasks.begin();
      for(;newTasks.size() > 0 && iter!=newTasks.end(); iter++){
        if((*iter)->isPSM()){ //PSM - actors need to be executed - even if Component can not execute tasks
          Task *newTask = *iter;
          this->taskTracer_->release(newTask);
          assert( readyTasks.find(newTask->getInstanceId())   == readyTasks.end()
                  /* A task can call compute only one time! */);
          assert( runningTasks.find(newTask->getInstanceId()) == runningTasks.end()
                  /* A task can call compute only one time! */);
          readyTasks[newTask->getInstanceId()]=newTask;
          scheduler->addedNewTask(newTask);
          newTasks.erase(iter);
          iter = newTasks.begin();
        }
      }
    }else{
      //look for new tasks (they called compute)
      if(disabledTasks.size()>0){
          std::deque<Task*>::iterator iter = disabledTasks.begin();
          for(;disabledTasks.size() > 0 && iter!=disabledTasks.end(); iter++){
            if((*iter)->getScheduledTask()->getActive()){
                newTasks.push_back(*iter);
                disabledTasks.erase(iter);
                iter = disabledTasks.begin();
            }
          }
      }

      while(newTasks.size()>0){
        Task *newTask;
        newTask=newTasks.front();
        TaskInterface* actor = newTask->getScheduledTask();
        newTasks.pop_front();
        if(actor!=NULL && !actor->getActive()){
            std::cout<<"actor disabled"<<std::endl;
            disabledTasks.push_back(newTask);
        }else{
          DBG_OUT(this->getName() << " received new Task: "
                  << newTask->getName() << " at: "
                  << sc_core::sc_time_stamp().to_default_time_units() << std::endl);
          this->taskTracer_->release(newTask);
          //insert new task in read list
          assert( readyTasks.find(newTask->getInstanceId())   == readyTasks.end()
                  /* A task can call compute only one time! */);
          assert( runningTasks.find(newTask->getInstanceId()) == runningTasks.end()
                  /* A task can call compute only one time! */);
          readyTasks[newTask->getInstanceId()]=newTask;
          scheduler->addedNewTask(newTask);
          }
      }
    }
  }

  /**
   *
   */
  void PreemptiveComponent::moveToRemainingPipelineStages(Task* task){
    sc_core::sc_time now                 = sc_core::sc_time_stamp();
    sc_core::sc_time restOfLatency       = task->getLatency()  - task->getDelay();
    sc_core::sc_time end                 = now + restOfLatency;
    if(end <= now){
      //early exit if (Latency-DII) <= 0
      //std::cerr << "Early exit: " << task->getName() << std::endl;
      this->taskTracer_->finishLatency(task);
      Director::getInstance().signalLatencyEvent(task);
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
        //<< front.pcb->getBlockEvent().latency << std::endl;
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

          this->taskTracer_->finishLatency(front.task);

          // Latency over -> remove Task
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
  void PreemptiveComponent::schedule_thread() {
    sc_core::sc_time timeslice;
    sc_core::sc_time actualRemainingDelay;
    sc_core::sc_time overhead = sc_core::SC_ZERO_TIME;
    int actualRunningIID;

    scheduler->initialize();
    fireStateChanged(ComponentState::IDLE);
//  std::string logName = getName();
//  logName = logName + ".buffer";
//  std::ofstream logBuffer(logName.c_str());
//  if( !logBuffer.is_open() ){
//    assert(false);
//  }
    unsigned int last_used_buffer = 0;
//  logBuffer << last_used_buffer << " " << sc_core::sc_time_stamp() << std::endl;

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
                  notify_scheduler_thread );
          }else{
            if(!pendingTask && !hasWaitingOrRunningTasks()){
              if(!requestShutdown()){
                notify_scheduler_thread.notify();
              }
            }
            wait( notify_scheduler_thread );
          }
        }else{                                        // a task already runs
          if(hasTimeSlice && (timeslice - overhead) < actualRemainingDelay){
            wait( timeslice - overhead,
                  notify_scheduler_thread );
          }else{
            wait( actualRemainingDelay,
                  notify_scheduler_thread );
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
            // all execution time simulated -> BLOCK running task.
            Task *task=runningTasks[actualRunningIID];

          DBG_OUT(this->getName() << " IID: " << actualRunningIID<< " > ");
          DBG_OUT(this->getName() << " removed Task: " << task->getName()
                  << " at: " << sc_core::sc_time_stamp().to_default_time_units()
                  << std::endl);

            //notify(*(task->blockEvent));
            scheduler->removedTask(task);
            fireStateChanged(ComponentState::IDLE);
            this->taskTracer_->finishDii(task);
            runningTasks.erase(actualRunningIID);

            task->getBlockEvent().dii->notify();

            if(multiCastGroups.size() != 0 && multiCastGroups.find(task->getProcessId()) != multiCastGroups.end()){
             for(std::list<MultiCastGroupInstance*>::iterator list_iter = multiCastGroupInstances.begin();
                         list_iter != multiCastGroupInstances.end(); list_iter++)
               {
                 MultiCastGroupInstance* mcgi = *list_iter;
                 if(mcgi->task == task){
                     for(std::list<Task*>::iterator tasks_iter = mcgi->additional_tasks->begin();
                         tasks_iter != mcgi->additional_tasks->end(); tasks_iter++){
                         (*tasks_iter)->getBlockEvent().dii->notify();
                         if((*tasks_iter)->hasScheduledTask()){
                           assert(((*tasks_iter)->getScheduledTask())->canFire());
                           ((*tasks_iter)->getScheduledTask())->scheduleLegacyWithCommState();
//                             assert(Director::canExecute((*tasks_iter)->getProcessId()));
//                             Director::execute((*tasks_iter)->getProcessId());
                         }
                         this->taskTracer_->finishDii((*tasks_iter));
                         this->taskTracer_->finishLatency((*tasks_iter));
                         Director::getInstance().signalLatencyEvent((*tasks_iter));
                     }
                     multiCastGroupInstances.remove(mcgi);
                     delete(mcgi->additional_tasks);
                     delete(mcgi);
                     break;
                 }
               }
            }

            if(task->hasScheduledTask()){
              assert((task->getScheduledTask())->canFire());
              (task->getScheduledTask())->scheduleLegacyWithCommState();
//                assert(Director::canExecute(task->getProcessId()));
//                Director::execute(task->getProcessId());
            }
            moveToRemainingPipelineStages(task);
            //wait(sc_core::SC_ZERO_TIME);
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
          notify_scheduler_thread.notify();
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

        /* */
        Task * assignedTask = runningTasks[taskToAssign];

        /*
         * Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
         */
        if (assignedTask->isPSM() == true){
            this->fireStateChanged(ComponentState::IDLE);
        }else{
            this->fireStateChanged(ComponentState::RUNNING);
        }

        if(assignedTask->isBlocking() /* && !assignedTask->isExec() */) {
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
      if(readyTasks.size() != last_used_buffer){
//      logBuffer<< readyTasks.size() << " " << sc_core::sc_time_stamp() << std::endl;
        last_used_buffer = readyTasks.size();
        if(last_used_buffer > max_used_buffer){
          max_used_buffer = readyTasks.size();
          //std::cout<<"MAX used Buffer of Component " << this->getName() << " increased to " << max_used_buffer << " @ " << sc_core::sc_time_stamp() << std::endl;
        }
      }
    }
//  // FIXME: Close is never reached cause of while!
//  logBuffer.close();
  }

} //namespace SystemC_VPC
