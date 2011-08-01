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

#include <systemcvpc/Scheduler.hpp>
#include <systemcvpc/FCFSScheduler.hpp>
#include <systemcvpc/TDMAScheduler.hpp>
#include <systemcvpc/FlexRayScheduler.hpp>
#include <systemcvpc/AVBScheduler.hpp>
#include <systemcvpc/RoundRobinScheduler.hpp>
#include <systemcvpc/PriorityScheduler.hpp>
#include <systemcvpc/RateMonotonicScheduler.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/TimeTriggeredCCScheduler.hpp>
#include <systemcvpc/PrioritySchedulerNoPreempt.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

#include "ComponentImpl.hpp"

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
      default:
        scheduler = new FCFSScheduler();
    }
  }

  /**
   *
   */
  void Component::compute(Task* actualTask){

    /* * /
    if(blockMutex > 0) {
      actualTask->abortBlockingCompute();
      return;
    }
    / * */

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

    //awake scheduler thread
    notify_scheduler_thread.notify();
    blockCompute.notify();
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

  Component::~Component(){
    this->setPowerConsumption(0.0);
    this->fireNotification(this);
#ifndef NO_POWER_SUM
    this->removeObserver(powerSumming);
    delete powerSumming;
    delete powerSumStream;
#endif // NO_POWER_SUM
  }

  void Component::notifyActivation(ScheduledTask * scheduledTask,
      bool active){
    if(active) {
      ttReleaseQueue.push(
          TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));

      assert(ttReleaseQueue.top().time>=sc_time_stamp());
      sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
      releaseActors.notify(delta);
    }
  }

  void Component::releaseActorsMethod(){
    TT::TimeNodePair tnp = ttReleaseQueue.top();
    ttReleaseQueue.pop();
    assert(tnp.time == sc_time_stamp());

    if(Director::canExecute(tnp.node)){
      Director::execute(tnp.node);
    }
    if(!ttReleaseQueue.empty()){
      assert(ttReleaseQueue.top().time>=sc_time_stamp());
      sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
      releaseActors.notify(delta);
    }
  }
} //namespace SystemC_VPC
