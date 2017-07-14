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

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>
#include <float.h>
#include <PreemptiveScheduler/AVBScheduler.hpp>
#include <PreemptiveScheduler/FlexRayScheduler.hpp>
#include <PreemptiveScheduler/MostScheduler.hpp>
#include <PreemptiveScheduler/MostSecondaryScheduler.hpp>
#include <PreemptiveScheduler/PriorityScheduler.hpp>
#include <PreemptiveScheduler/RateMonotonicScheduler.hpp>
#include <PreemptiveScheduler/RoundRobinScheduler.hpp>
#include <PreemptiveScheduler/Scheduler.hpp>
#include <PreemptiveScheduler/StreamShaperScheduler.hpp>
#include <PreemptiveScheduler/TDMAScheduler.hpp>
#include <PreemptiveScheduler/TimeTriggeredCCScheduler.hpp>
#include <PreemptiveScheduler/PreemptiveComponent.hpp>

namespace SystemC_VPC{

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
    pendingTask = false;
    if(max_avail_buffer == 0 || (getReadyTasks().size() + newTasks.size() + tasksDuringNoExecutionPhase.size()) < max_avail_buffer){
      ProcessId pid = actualTask->getProcessId();
      ProcessControlBlockPtr pcb = this->getPCB(pid);
      actualTask->setPCB(pcb);
      actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

      DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
              << " ) at time: " << sc_time_stamp()
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
      //std::cout<< "Message/Task " << actualTask->getName() << " dropped due to less buffer-space of component "<< getName() << " @ " << sc_time_stamp() << std::endl;
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
      if(ttReleaseQueue.top().time<=sc_time_stamp()){
        releaseActors.notify();
      }else{
       sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
       releaseActors.notify(delta);
      }
    }
    //awake scheduler thread
    notify_scheduler_thread.notify();
    blockCompute.notify();
  }

  void PreemptiveComponent::notifyActivation(ScheduledTask * scheduledTask,
      bool active){
    if(active) {
      TT::TimeNodePair newTask = TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask);
      //std::cout<<"Component " << this->getName() << " notifyActivation("<<scheduledTask->getPid()<<", " << (this->getPCB(scheduledTask->getPid()))->getName() << " isPSM=" << this->getPCB(scheduledTask->getPid())->isPSM() << " @ " << newTask.time << " @ " << sc_time_stamp() << std::endl;
      if(this->getCanExecuteTasks() || this->getPCB(scheduledTask->getPid())->isPSM()){
        pendingTask = true;
        ttReleaseQueue.push(newTask);

        if(ttReleaseQueue.top().time<=sc_time_stamp()){
          releaseActors.notify(SC_ZERO_TIME);
        }else{
         sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
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
    //std::cout<<"Component " << this->getName() << " releaseActorsMethod " << ttReleaseQueue.top().node->getPid() << " @ " << sc_time_stamp() << std::endl;
      TT::TimeNodePair tnp = ttReleaseQueue.top();
      if(tnp.time <= sc_time_stamp()){
        ttReleaseQueue.pop();
        assert(tnp.time <= sc_time_stamp());
        if(this->getCanExecuteTasks() || this->getPCB(tnp.node->getPid())->isPSM()){
          if(tnp.node->canFire()){
            tnp.node->schedule();
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
        if(ttReleaseQueue.top().time<=sc_time_stamp()){
          // The SC_ZERO_TIME is needed, otherwise releaseActorsMethod won't
          // be recalled.
          releaseActors.notify(SC_ZERO_TIME);
        }else{
          assert(ttReleaseQueue.top().time>=sc_time_stamp());
          sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
          releaseActors.notify(delta);
        }
      }
  }
} //namespace SystemC_VPC
