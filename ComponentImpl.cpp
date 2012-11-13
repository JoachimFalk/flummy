/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>
#include "ComponentImpl.hpp"
#include "scheduler/AVBScheduler.hpp"
#include <scheduler/MostScheduler.hpp>
#include <scheduler/StreamShaperScheduler.hpp>
#include <scheduler/MostSecondaryScheduler.hpp>
#include "scheduler/FlexRayScheduler.hpp"
#include "scheduler/FCFSScheduler.hpp"
#include "scheduler/PriorityScheduler.hpp"
#include "scheduler/PrioritySchedulerNoPreempt.hpp"
#include "scheduler/RateMonotonicScheduler.hpp"
#include "scheduler/RoundRobinScheduler.hpp"
#include "scheduler/Scheduler.hpp"
#include "scheduler/TimeTriggeredCCScheduler.hpp"
#include "scheduler/TDMAScheduler.hpp"

#include <float.h>

namespace SystemC_VPC{

  /**
   *
   */
  void Component::setScheduler(Config::Component::Ptr component)
  {
    Config::Scheduler::Type type = component->getScheduler();
    switch (type) {
      case Config::Scheduler::RoundRobin:
        scheduler = new RoundRobinScheduler();
        break;
      case Config::Scheduler::StaticPriority_NP:
        scheduler = new PrioritySchedulerNoPreempt();
        break;
      case Config::Scheduler::StaticPriority_P:
        scheduler = new PriorityScheduler();
        break;
      case Config::Scheduler::RateMonotonic:
        scheduler = new RateMonotonicScheduler();
        break;
      case Config::Scheduler::FCFS_old:
        scheduler = new FCFSScheduler();
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
        scheduler = new FCFSScheduler();
    }
  }

  /**
   *
   */
  void Component::compute(Task* actualTask){
    if(multiCastGroups.size() != 0 && multiCastGroups.find(actualTask->getProcessId()) != multiCastGroups.end()){
        //MCG vorhanden und Task auch als MultiCast zu behandeln
        MultiCastGroupInstance* instance = getMultiCastGroupInstance(actualTask);

        //handling of buffer overflow
        actualTask->getBlockEvent().latency->setDropped(instance->task->getBlockEvent().latency->getDropped());
        if(instance->task != actualTask){
            //instance already running...
            //start tracing etc. TODO!
            return;
        }
    }
    pendingTask = false;
    if(max_avail_buffer == 0 || (readyTasks.size() + newTasks.size() + tasksDuringNoExecutionPhase.size()) < max_avail_buffer){
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
  void Component::requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker){
    task->setExec(false);
    task->setBlockingCompute( blocker );
    this->compute( task );
  }

  /**
   *
   */
  void Component::execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker){
    task->setExec(true);
    blockCompute.notify();
  }


  /**
   *
   */
  void Component::abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker){
    task->resetBlockingCompute();
    blockCompute.notify();
  }


  void Component::updatePowerConsumption()
  {
    this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
    // Notify observers (e.g. powersum)
    this->fireNotification(this);
  }

  void Component::fireStateChanged(const ComponentState &state)
  {
    this->setComponentState(state);
    this->updatePowerConsumption();
  }

  void Component::initialize(const Director* d){
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

  bool Component::setAttribute(AttributePtr attribute){
    bool isComponentAttribute = AbstractComponent::setAttribute(attribute);
    if (!isComponentAttribute){
      scheduler->setAttribute(attribute);
    }
    return true;
  }
  bool Component::addStream(ProcessId pid){
    return scheduler->addStream(pid);
  }

  bool Component::closeStream(ProcessId pid){
    return scheduler->closeStream(pid);
  }


  Component::~Component(){
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
  void Component::reactivateExecution(){
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

  void Component::notifyActivation(ScheduledTask * scheduledTask,
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

  void Component::releaseActorsMethod(){
//     if(this->getCanExecuteTasks()){ //not required, no "normal" task will be added to ttReleaseQueue
    //std::cout<<"Component " << this->getName() << " releaseActorsMethod " << ttReleaseQueue.top().node->getPid() << " @ " << sc_time_stamp() << std::endl;
      TT::TimeNodePair tnp = ttReleaseQueue.top();
      sc_time curr = sc_time_stamp();

      if(tnp.time <= sc_time_stamp()){
        ttReleaseQueue.pop();
        assert(tnp.time <= sc_time_stamp());
        if(this->getCanExecuteTasks() || this->getPCB(tnp.node->getPid())->isPSM()){
          if(Director::canExecute(tnp.node)){
            Director::execute(tnp.node);
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
          releaseActors.notify();
        }else{
          assert(ttReleaseQueue.top().time>=sc_time_stamp());
          sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
          releaseActors.notify(delta);
        }
      }
  }
} //namespace SystemC_VPC
