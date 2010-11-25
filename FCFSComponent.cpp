/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * Component.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#include <systemcvpc/FCFSComponent.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>

#include <float.h>

namespace SystemC_VPC{

  /**
   *
   */
  FCFSComponent::FCFSComponent( std::string name, Director *director )
      : AbstractComponent(name.c_str()),
        runningTask(NULL),
        blockMutex(0)
    {
      SC_METHOD(schedule_method);
      sensitive << notify_scheduler_thread;
      dont_initialize();

      //SC_THREAD(remainingPipelineStages);

      this->setPowerMode(this->translatePowerMode("SLOW"));

      midPowerGov = new InternalLoadHysteresisGovernor(
        sc_time(12.5, SC_MS),
        sc_time(12.1, SC_MS),
        sc_time( 4.0, SC_MS));
      midPowerGov->setGlobalGovernor(director->topPowerGov);


      if(powerTables.find(getPowerMode()) == powerTables.end()){
        powerTables[getPowerMode()] = PowerTable();
      }

      PowerTable &powerTable=powerTables[getPowerMode()];
      powerTable[ComponentState::IDLE]    = 0.0;
      powerTable[ComponentState::RUNNING] = 1.0;

#ifndef NO_POWER_SUM
      std::string powerSumFileName(this->getName());
      powerSumFileName += ".dat";

      powerSumStream = new std::ofstream(powerSumFileName.c_str());
      powerSumming   = new PowerSumming(*powerSumStream);
      this->addObserver(powerSumming);
#endif // NO_POWER_SUM

      fireStateChanged(ComponentState::IDLE);
    }


  /**
   *
   */
  void FCFSComponent::schedule_method(){
    DBG_OUT("FCFSComponent::schedule_method (" << this->name()
            << ") triggered @" << sc_time_stamp() << endl);

    //default trigger
    next_trigger(notify_scheduler_thread);

    if (runningTask == NULL) {
      bool released = releaseActor();
      if (released){
        //assert(!readyTasks.empty());
      }

      if (!readyTasks.empty()){
        scheduleTask();
        next_trigger(runningTask->getRemainingDelay());

      }

    } else {
      assert(startTime+runningTask->getRemainingDelay() <= sc_time_stamp());
      removeTask();
      schedule_method(); //recursion will release/schedule next task
    }
  }


  /**
   *
   */
  void  FCFSComponent::setScheduler(const char *schedulername){
  }

  /**
   *
   */
  void FCFSComponent::compute(Task* actualTask){

    /* * /
    if(blockMutex > 0) {
      actualTask->abortBlockingCompute();
      return;
    }
    / * */

    ProcessId pid = actualTask->getProcessId();
    PCBPool &pool = this->getPCBPool();
    assert(pool.find(pid) != pool.end());
    ProcessControlBlockPtr pcb = pool[pid];
    actualTask->setPCB(pcb);
    actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
            << " ) at time: " << sc_time_stamp()
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

    //awake scheduler thread
    if(runningTask == NULL && !actualTask->hasScheduledTask()){
      notify_scheduler_thread.notify();
      //blockCompute.notify();
    }
  }



  /**
   *
   */
  void FCFSComponent::requestBlockingCompute(Task* task,
                                             RefCountEventPtr blocker){
    task->setExec(false);
    task->setBlockingCompute( blocker );
    this->compute( task );
  }

  /**
   *
   */
  void FCFSComponent::execBlockingCompute(Task* task, RefCountEventPtr blocker){
    task->setExec(true);
    blockCompute.notify();
  }


  /**
   *
   */
  void FCFSComponent::abortBlockingCompute(Task* task, RefCountEventPtr blocker){
    task->resetBlockingCompute();
    blockCompute.notify();
  }

  /**
   *
   */
  void FCFSComponent::remainingPipelineStages(){
    while(1){
      if(pqueue.size() == 0){
        wait(remainingPipelineStages_WakeUp);
      }else{
        timePcbPair front = pqueue.top();

        //cerr << "Pop from list: " << front.time << " : " 
        //<< front.pcb->getBlockEvent().latency << endl;
        sc_time waitFor = front.time-sc_time_stamp();
        assert(front.time >= sc_time_stamp());
        //cerr << "Pipeline> Wait till " << front.time
        //<< " (" << waitFor << ") at: " << sc_time_stamp() << endl;
        wait( waitFor, remainingPipelineStages_WakeUp );

        sc_time rest = front.time-sc_time_stamp();
        assert(rest >= SC_ZERO_TIME);
        if(rest > SC_ZERO_TIME){
          //cerr << "------------------------------" << endl;
        }else{
          assert(rest == SC_ZERO_TIME);
          //cerr << "Ready! releasing task (" <<  front.time <<") at: "
          //<< sc_time_stamp() << endl;

          // Latency over -> remove Task
          Director::getInstance().signalLatencyEvent(front.task);

          //wait(SC_ZERO_TIME);
          pqueue.pop();
        }
      }

    }
  }

  /**
   *
   */
  void FCFSComponent::moveToRemainingPipelineStages(Task* task){
    sc_time now                 = sc_time_stamp();
    sc_time restOfLatency       = task->getLatency()  - task->getDelay();
    sc_time end                 = now + restOfLatency;
    if(end <= now){
      //early exit if (Latency-DII) <= 0
      //std::cerr << "Early exit: " << task->getName() << std::endl;
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

  void FCFSComponent::updatePowerConsumption()
  {
    this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
    // Notify observers (e.g. powersum)
    this->fireNotification(this);
  }

  void FCFSComponent::fireStateChanged(const ComponentState &state)
  {
    this->setComponentState(state);
    this->updatePowerConsumption();
  }

  void FCFSComponent::notifyActivation(ScheduledTask * scheduledTask,
      bool active){
    DBG_OUT(this->name() << " notifyActivation " << scheduledTask
        << " " << active << std::endl);
    if (active) {
      fcfsQueue.push_back(scheduledTask);
      if (runningTask == NULL) {
        notify_scheduler_thread.notify(SC_ZERO_TIME);
      }
    }
  }

} //namespace SystemC_VPC
