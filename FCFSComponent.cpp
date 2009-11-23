/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_Component.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#include <systemcvpc/FCFSComponent.h>
//#include "hscd_vpc_Scheduler.h>
#include <systemcvpc/hscd_vpc_datatypes.h>
#include <systemcvpc/Task.h>

#include <float.h>

namespace SystemC_VPC{

  /**
   *
   */
  void FCFSComponent::schedule_method(){
    DBG_OUT("FCFSComponent::schedule_method (" << this->name()
            << ") triggered @" << sc_time_stamp() << endl);

    if(runningTask != NULL) {
      if(startTime+runningTask->getRemainingDelay() <= sc_time_stamp()){
        // remove running task
        removeTask();
        // assign new task
        if(!readyTasks.empty()) {
          scheduleTask();
          next_trigger(runningTask->getRemainingDelay(),
                       notify_scheduler_thread);
        }else {
          next_trigger(notify_scheduler_thread);
        }
      }else{
        // blocking compute iterrupt ??
        DBG_OUT("blocking compute iterrupt ??" << endl);
      }

    }else{
      scheduleTask();
      next_trigger(runningTask->getRemainingDelay(),
                   notify_scheduler_thread);
    }
  }


  bool FCFSComponent::processPower(Attribute att)
  {
    // hierarchical format
    if(!att.isType("powermode")) {
      return false;
    }
    
    this->addPowerGovernor(midPowerGov);

    for(size_t i=0; i<att.getAttributeSize();++i){
      Attribute powerAtt = att.getNextAttribute(i).second;
      if(powerAtt.isType("governor")){
        sc_time window    = sc_time(12.5, SC_MS);
        sc_time upperTime = sc_time(12.1, SC_MS);
        sc_time lowerTime = sc_time( 4.0, SC_MS);
        if(powerAtt.hasParameter("sliding_window")){
          std::string v = powerAtt.getParameter("sliding_window");
          window = Director::createSC_Time(v.c_str());
        }
        if(powerAtt.hasParameter("upper_threshold")){
          std::string v = powerAtt.getParameter("upper_threshold");
          upperTime = window*atof(v.c_str());
        }
        if(powerAtt.hasParameter("lower_threshold")){
          std::string v = powerAtt.getParameter("lower_threshold");
          lowerTime = window*atof(v.c_str());
        }
        /* FIXME:
        midPowerGov->setParams(window,
                               upperTime,
                               lowerTime);
        */
        continue;
      }



      std::string powerMode = att.getNextAttribute(i).first;
      const PowerMode *power = this->translatePowerMode(powerMode);

      if(powerTables.find(power) == powerTables.end()){
        powerTables[power] = PowerTable();
      }

      PowerTable &powerTable=powerTables[power];

      if(powerAtt.hasParameter("IDLE")){
        std::string v = powerAtt.getParameter("IDLE");
        const double value = atof(v.c_str());
        powerTable[ComponentState::IDLE] = value;
      }
      if(powerAtt.hasParameter("RUNNING")){
        std::string v = powerAtt.getParameter("RUNNING");
        const double value = atof(v.c_str());
        powerTable[ComponentState::RUNNING] = value;
      }
      if(powerAtt.hasParameter("STALLED")){
        std::string v = powerAtt.getParameter("STALLED");
        const double value = atof(v.c_str());
        powerTable[ComponentState::STALLED] = value;
      }
      if(powerAtt.hasParameter("transaction_delay")) {
        this->transactionDelays[power] =
          Director::createSC_Time(powerAtt.getParameter("transaction_delay"));
      }

    }
        
    return true;
  }
  
  void FCFSComponent::setAttribute(Attribute& attributes){
    if(processPower(attributes)){
      return;
    }

    if(attributes.isType("transaction_delay")) {
      this->transactionDelays[this->getPowerMode()] =
        Director::createSC_Time(attributes.getValue());
      return;
    }
    
    if(attributes.isType("transaction")) {
      unsigned int transactionSize = 1;
      sc_time transactionDelay     = SC_ZERO_TIME;
      if(attributes.hasParameter("delay")){
        transactionDelay =
          Director::createSC_Time(attributes.getParameter("delay"));
      }

      if(attributes.hasParameter("size")){
        transactionSize = atoi(attributes.getParameter("size").c_str());
      }

      this->transactionDelays[this->getPowerMode()] = transactionDelay;
      // FIXME: add transactionSize
      return;
    }
    
    //scheduler->setAttribute(attributes);
  }

  /**
   *
   */
  void  FCFSComponent::setScheduler(const char *schedulername){
  }

  /**
   *
   */
  void FCFSComponent::informAboutMapping(std::string module){
#ifndef NO_VCD_TRACES
    Tracing *newsignal = new Tracing();
    trace_map_by_name.insert(std::pair<std::string,Tracing* >(module, newsignal));
    sc_trace(this->traceFile, *newsignal->traceSignal, module.c_str());
    newsignal->traceSleeping();
#endif //NO_VCD_TRACES

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
    ProcessControlBlock* pcb = pool[pid];
    actualTask->setPCB(pcb);
    actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
            << " ) at time: " << sc_time_stamp()
            << " mode: " << this->getPowerMode()->getName()
            << std::endl);

    // reset the execution delay
    actualTask->initDelays();
    DBG_OUT("Using " << actualTask->getRemainingDelay()
         << " as delay for function " << actualTask->getFunctionId() << "!"
         << std::endl);
    DBG_OUT("And " << actualTask->getLatency() << " as latency for function "
         << actualTask->getFunctionId() << "!" << std::endl);
    
#ifndef NO_VCD_TRACES
    {
      std::map<std::string, Tracing* >::iterator iter
        = trace_map_by_name.find(actualTask->getName());
      if( iter != trace_map_by_name.end() ){
        actualTask->setTraceSignal(iter->second);
      }
    }
#endif //NO_VCD_TRACES

    //store added task
    this->addTask(actualTask);

    //awake scheduler thread
    if(runningTask == NULL){
      notify_scheduler_thread.notify();
      //blockCompute.notify();
    }
  }



  /**
   *
   */
  void FCFSComponent::requestBlockingCompute(Task* task, Event* blocker){
    task->setExec(false);
    task->setBlockingCompute( blocker );
    this->compute( task );
  }

  /**
   *
   */
  void FCFSComponent::execBlockingCompute(Task* task, Event* blocker){
    task->setExec(true);
    blockCompute.notify();
  }


  /**
   *
   */
  void FCFSComponent::abortBlockingCompute(Task* task, Event* blocker){
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
          this->notifyParentController(front.task);

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
      this->notifyParentController(task);
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

} //namespace SystemC_VPC
