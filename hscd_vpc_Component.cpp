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
#include <hscd_vpc_Component.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_FCFSScheduler.h>
#include <TDMAScheduler.h>
#include <FlexRayScheduler.h>
#include <TimeTriggeredCCScheduler.h>
#include <hscd_vpc_RoundRobinScheduler.h>
#include <hscd_vpc_PriorityScheduler.h>
#include <hscd_vpc_RateMonotonicScheduler.h>
#include <hscd_vpc_datatypes.h>
#include "Task.h"

#include <float.h>

#include "debug_config.h"
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_COMPONENT
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

namespace SystemC_VPC{

  /**
   *
   */
  void Component::schedule_thread(){
    sc_time timeslice;
    sc_time actualRemainingDelay;
    sc_time *overhead = new sc_time( SC_ZERO_TIME );
    int actualRunningIID;
    bool newTaskDuringOverhead=false;
    //wait(SC_ZERO_TIME);
    
    scheduler->initialize();
    fireStateChanged(ComponentState::IDLE);
    
    while(1){
      // Notify observers (e.g. powersum)
      this->fireNotification(this);

      //determine the time slice for next scheduling descission and wait for
      bool hasTimeSlice= scheduler->getSchedulerTimeSlice( timeslice,
                                                           readyTasks,
                                                           runningTasks );
      startTime = sc_time_stamp();
      if(!newTaskDuringOverhead){ 
        if(runningTasks.size()<=0){                    // no running task
          if(hasTimeSlice){                           
            wait( timeslice - (*overhead),
                  notify_scheduler_thread ); 
          }else{
            wait( notify_scheduler_thread );
          }
        }else{                                        // a task allready runs
          if(hasTimeSlice && (timeslice - (*overhead)) < actualRemainingDelay){
            wait( timeslice - (*overhead),
                  notify_scheduler_thread );
          }else{
            wait( actualRemainingDelay,
                  notify_scheduler_thread );
          }
          sc_time runTime=sc_time_stamp()-startTime;
          assert(runTime.value()>=0);
          actualRemainingDelay-=runTime;

          assert(actualRemainingDelay.value()>=0);

          DBG_OUT("Component " << this->getName()
                    << "> actualRemainingDelay= "
                    << actualRemainingDelay.value() << " for iid="
                    << actualRunningIID << " at: "
                    << sc_time_stamp().to_default_time_units()
                    << std::endl);

          if(actualRemainingDelay.value()==0){
            // all execution time simulated -> BLOCK running task.
            Task *task=runningTasks[actualRunningIID];

          DBG_OUT(this->getName() << " IID: " << actualRunningIID<< " > ");
          DBG_OUT(this->getName() << " removed Task: " << task->getName()
                  << " at: " << sc_time_stamp().to_default_time_units()
                  << std::endl);

            //notify(*(task->blockEvent));
            scheduler->removedTask(task);
            fireStateChanged(ComponentState::IDLE);
#ifndef NO_VCD_TRACES
            if(task->getTraceSignal()!=0)
              task->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
            runningTasks.erase(actualRunningIID);

            task->getBlockEvent().dii->notify();
            moveToRemainingPipelineStages(task);
            //wait(SC_ZERO_TIME);
          }else{

            // store remainingDelay
            runningTasks[actualRunningIID]->setRemainingDelay(
              actualRemainingDelay );
          }
        }
      }else{
        newTaskDuringOverhead=false;
      }

      //look for new tasks (they called compute)
      while(newTasks.size()>0){
        Task *newTask;
        newTask=newTasks[0];
        newTasks.pop_front();
        DBG_OUT(this->getName() << " received new Task: "
             << newTask->getName() << " at: "
             << sc_time_stamp().to_default_time_units() << std::endl);
#ifndef NO_VCD_TRACES
        if(newTask->getTraceSignal()!=0)
          newTask->getTraceSignal()->traceReady();
#endif //NO_VCD_TRACES
        //insert new task in read list
        assert( readyTasks.find(newTask->getInstanceId())   == readyTasks.end()
                /* A task can call compute only one time! */);
        assert( runningTasks.find(newTask->getInstanceId()) ==
                runningTasks.end()
                /* A task can call compute only one time! */);

        readyTasks[newTask->getInstanceId()]=newTask;
        scheduler->addedNewTask(newTask);
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
#ifndef NO_VCD_TRACES
        if(readyTasks[taskToResign]->getTraceSignal()!=0)
          readyTasks[taskToResign]->getTraceSignal()->traceReady();
#endif //NO_VCD_TRACES
      }

      sc_time timestamp=sc_time_stamp();
      if( overhead != NULL ) delete overhead;
      overhead=scheduler->schedulingOverhead();

      if( overhead != NULL ){
#ifndef NO_VCD_TRACES
        schedulerTrace =  SystemC_VPC::Tracing::S_RUNNING;
#endif //NO_VCD_TRACES
        //    actual time    < endtime
        while( (sc_time_stamp() < timestamp + (*overhead)) ){ 

          wait( (timestamp+(*overhead))-sc_time_stamp(),
                notify_scheduler_thread );

        }

        // true if some task becames ready during overhead waiting
        newTaskDuringOverhead=(newTasks.size()>0);
#ifndef NO_VCD_TRACES
        schedulerTrace = SystemC_VPC::Tracing::S_READY;
#endif //NO_VCD_TRACES
      }else {
        // avoid failures
        overhead=new sc_time(SC_ZERO_TIME);
      }


      //assign task
      if(decision==ONLY_ASSIGN || decision==PREEMPT){
        runningTasks[taskToAssign]=readyTasks[taskToAssign];
#ifndef NO_VCD_TRACES
        if(runningTasks[taskToAssign]->getTraceSignal()!=0)
          runningTasks[taskToAssign]->getTraceSignal()->traceRunning();
#endif //NO_VCD_TRACES
        readyTasks.erase(taskToAssign);
        actualRunningIID=taskToAssign;
        DBG_OUT("IID: " << taskToAssign << "> remaining delay for "
             << runningTasks[taskToAssign]->getName());
        actualRemainingDelay 
          = sc_time(runningTasks[taskToAssign]->getRemainingDelay());
        DBG_OUT(" is " << runningTasks[taskToAssign]->getRemainingDelay()
             << endl);
        fireStateChanged(ComponentState::RUNNING);

        /* */
        Task * assignedTask = runningTasks[taskToAssign];
        if(assignedTask->isBlocking() /* && !assignedTask->isExec() */) {
          blockMutex++;
          if(blockMutex == 1) {
            DBG_OUT(this->getName() << " scheduled blocking task: "
                    << assignedTask->getName() << std::endl);
            assignedTask->ackBlockingCompute();
            DBG_OUT(this->getName() << " enter wait: " << std::endl);
            fireStateChanged(ComponentState::STALLED);
#ifndef NO_VCD_TRACES
            if(assignedTask->getTraceSignal()!=0)
              assignedTask->getTraceSignal()->traceBlocking();
#endif //NO_VCD_TRACES
            while(!assignedTask->isExec()){
              CoSupport::SystemC::wait(blockCompute);
              blockCompute.reset();
            }
            DBG_OUT(this->getName() << " exit wait: " << std::endl);
            fireStateChanged(ComponentState::RUNNING);
#ifndef NO_VCD_TRACES
            if(assignedTask->getTraceSignal()!=0)
              assignedTask->getTraceSignal()->traceRunning();
#endif //NO_VCD_TRACES
            if(assignedTask->isBlocking()){
              DBG_OUT(this->getName() << " exec Task: "
                      << assignedTask->getName() << " @  " << sc_time_stamp()
                      << std::endl);
              // task is still blocking: exec task
            } else {
              DBG_OUT(this->getName() << " abort Task: "
                      << assignedTask->getName() << " @  " << sc_time_stamp()
                      << std::endl);

              //notify(*(task->blockEvent));
              scheduler->removedTask(assignedTask);
              fireStateChanged(ComponentState::IDLE);
#ifndef NO_VCD_TRACES
              if(assignedTask->getTraceSignal()!=0)
                assignedTask->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
              runningTasks.erase(actualRunningIID);
             
            }
          }else{
            assert(blockMutex>1);
            scheduler->removedTask(assignedTask);
            fireStateChanged(ComponentState::IDLE);
#ifndef NO_VCD_TRACES
            if(assignedTask->getTraceSignal()!=0)
              assignedTask->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
            runningTasks.erase(actualRunningIID);
            assignedTask->abortBlockingCompute();
          }
          blockMutex--;
        }
        /* */
      }
    }

  }


  bool Component::processPower(Attribute att)
  {
    // hierarchical format
    if(!att.isType("powermode")) {
      return false;
    }

    for(size_t i=0; i<att.getAttributeSize();++i){
      Attribute powerAtt = att.getNextAttribute(i).second;
      std::string powerMode = att.getNextAttribute(i).first;
      PowerMode power = this->translatePowerMode(powerMode);

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
    }
        
    return true;
  }

  /**
   *
   */
  void Component::processAndForwardParameter(char *sType,char *sValue){
    scheduler->setProperty(sType,sValue);
  }
  
  void Component::setAttribute(Attribute& attributes){
    if(processPower(attributes)){
      return;
    }
    scheduler->setAttribute(attributes);
  }

  /**
   *
   */
  void  Component::setScheduler(const char *schedulername){
    if( 0==strncmp(schedulername,STR_ROUNDROBIN,strlen(STR_ROUNDROBIN))
        || 0==strncmp(schedulername,STR_RR,strlen(STR_RR)) ){
      scheduler=new RoundRobinScheduler((const char*)schedulername);

    }else if( 0==strncmp(schedulername,
                         STR_PRIORITYSCHEDULER,strlen(STR_PRIORITYSCHEDULER))
              || 0==strncmp(schedulername,STR_PS,strlen(STR_PS)) ){
      scheduler=new PriorityScheduler((const char*)schedulername);

    }else if( 0==strncmp(schedulername,STR_RATEMONOTONIC,
                         strlen(STR_RATEMONOTONIC))
              || 0==strncmp(schedulername,STR_RM,strlen(STR_RM))){
      scheduler=new RateMonotonicScheduler((const char*)schedulername);

    }else if( 0==strncmp(schedulername,STR_FIRSTCOMEFIRSTSERVE,
                         strlen(STR_FIRSTCOMEFIRSTSERVE))
              || 0==strncmp(schedulername,STR_FCFS,strlen(STR_FCFS))){
      scheduler=new FCFSScheduler();
      
      // TDMA hat nur einen Namen ;-)
    }else if( 0==strncmp(schedulername,STR_TDMA,
                         strlen(STR_TDMA))){
      scheduler=new TDMAScheduler((const char*)schedulername);

    }else if( 0==strncmp(schedulername,STR_FLEXRAY,
                         strlen(STR_FLEXRAY))){
      scheduler=new FlexRayScheduler((const char*)schedulername);
    }else if( 0==strncmp(schedulername,STR_TTCC,
                         strlen(STR_TTCC))){
      scheduler=new TimeTriggeredCCScheduler((const char*)schedulername);
    }else{
      //    cerr << "Scheduler: "<< STR_FIRSTCOMEFIRSTSERVE << endl;
      scheduler=new FCFSScheduler();
    }
  }

  /**
   *
   */
  void Component::informAboutMapping(std::string module){
#ifndef NO_VCD_TRACES
    Tracing *newsignal = new Tracing();
    trace_map_by_name.insert(std::pair<std::string,Tracing* >(module, newsignal));
    sc_trace(this->traceFile, *newsignal->traceSignal, module.c_str());
#endif //NO_VCD_TRACES

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
    PCBPool &pool = this->getPCBPool();
    assert(pool.find(pid) != pool.end());
    ProcessControlBlock* pcb = pool[pid];
    actualTask->setPCB(pcb);
    actualTask->setTiming(this->getTiming(*this->getPowerMode(), pid));

    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
            << " ) at time: " << sc_time_stamp()
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
    newTasks.push_back(actualTask);

    //awake scheduler thread
    notify_scheduler_thread.notify();
  }



  /**
   *
   */
  void Component::requestBlockingCompute(Task* task, Event* blocker){
    task->setExec(false);
    task->setBlockingCompute( blocker );
    this->compute( task );
  }

  /**
   *
   */
  void Component::execBlockingCompute(Task* task, Event* blocker){
    task->setExec(true);
    blockCompute.notify();
  }


  /**
   *
   */
  void Component::abortBlockingCompute(Task* task, Event* blocker){
    task->resetBlockingCompute();
    blockCompute.notify();
  }

  /**
   *
   */
  void Component::remainingPipelineStages(){
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
  void Component::moveToRemainingPipelineStages(Task* task){
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

  void Component::fireStateChanged(const ComponentState &state)
  {
    this->setComponentState(state);
    this->setPowerConsumption(powerTables[*getPowerMode()][state]);
  }
} //namespace SystemC_VPC
