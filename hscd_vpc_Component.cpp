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
#include <hscd_vpc_Director.h>

#include <float.h>

#include "debug_config.h"
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_COMPONENT
#include <cosupport/smoc_debug_out.hpp>
#include <cosupport/filter_ostream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

namespace SystemC_VPC{

  const ComponentState Component::IDLE    = 0;
  const ComponentState Component::RUNNING = 1;

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
    setComponentState(IDLE);
    
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

#ifdef VPC_DEBUG
          std::cerr << VPC_RED("Component " << this->getName()
                    << "> actualRemainingDelay= "
                    << actualRemainingDelay.value() << " for iid="
                    << actualRunningIID << " at: "
                    << sc_time_stamp().to_default_time_units())
                    << std::endl;
#endif //VPC_DEBUG

          if(actualRemainingDelay.value()==0){
            // all execution time simulated -> BLOCK running task.
            ProcessControlBlock *task=runningTasks[actualRunningIID];

            task->setState(ending);
            Director::getInstance().checkConstraints();
            task->setState(inaktiv);

#ifdef VPC_DEBUG
            cerr << this->getName() << " IID: " << actualRunningIID<< " > ";
            cerr << this->getName() << " removed Task: " << task->getName()
                 << " at: " << sc_time_stamp().to_default_time_units() << endl;
#endif // VPCDEBUG

            //notify(*(task->blockEvent));
            scheduler->removedTask(task);
            setComponentState(IDLE);
#ifndef NO_VCD_TRACES
            if(task->getTraceSignal()!=0)
              task->getTraceSignal()->value(S_BLOCKED);     
#endif //NO_VCD_TRACES
            runningTasks.erase(actualRunningIID);

            task->getBlockEvent().dii->notify();
            moveToRemainingPipelineStages(task);
            //wait(SC_ZERO_TIME);
          }else{

            // store remainingDela within ProcessControlBlock
            runningTasks[actualRunningIID]->setRemainingDelay(
              actualRemainingDelay );
          }
        }
      }else{
        newTaskDuringOverhead=false;
      }

      //look for new tasks (they called compute)
      while(newTasks.size()>0){
        ProcessControlBlock *newTask;
        newTask=newTasks[0];
        newTasks.pop_front();
#ifdef VPC_DEBUG
        cerr << this->getName() << " received new Task: "
             << newTask->getName() << " at: "
             << sc_time_stamp().to_default_time_units() << endl;
#endif // VPCDEBUG
#ifndef NO_VCD_TRACES
        if(newTask->getTraceSignal()!=0)
          newTask->getTraceSignal()->value(S_READY);     
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
        setComponentState(IDLE);
#ifndef NO_VCD_TRACES
        if(readyTasks[taskToResign]->getTraceSignal()!=0)
          readyTasks[taskToResign]->getTraceSignal()->value(S_READY);     
#endif //NO_VCD_TRACES
      }

      sc_time timestamp=sc_time_stamp();
      if( overhead != NULL ) delete overhead;
      overhead=scheduler->schedulingOverhead();

      if( overhead != NULL ){
        schedulerTrace = S_RUNNING;
        //    actual time    < endtime
        while( (sc_time_stamp() < timestamp + (*overhead)) ){ 

          wait( (timestamp+(*overhead))-sc_time_stamp(),
                notify_scheduler_thread );

        }

        // true if some task becames ready during overhead waiting
        newTaskDuringOverhead=(newTasks.size()>0);
        schedulerTrace=S_READY;
      }else {
        // avoid failures
        overhead=new sc_time(SC_ZERO_TIME);
      }


      //assign task
      if(decision==ONLY_ASSIGN || decision==PREEMPT){
        runningTasks[taskToAssign]=readyTasks[taskToAssign];
        readyTasks.erase(taskToAssign);
        actualRunningIID=taskToAssign;
#ifdef VPC_DEBUG
        cerr << "IID: " << taskToAssign << "> remaining delay for "
             << runningTasks[taskToAssign]->getName();
#endif // VPCDEBUG
        actualRemainingDelay 
          = sc_time(runningTasks[taskToAssign]->getRemainingDelay());
#ifdef VPC_DEBUG
        cerr << " is " << runningTasks[taskToAssign]->getRemainingDelay()
             << endl;
#endif // VPCDEBUG
        setComponentState(RUNNING);
#ifndef NO_VCD_TRACES
        if(runningTasks[taskToAssign]->getTraceSignal()!=0)
          runningTasks[taskToAssign]->getTraceSignal()->value(S_RUNNING);     
#endif //NO_VCD_TRACES
      }
    }

  }


  bool Component::processParameter(char *sType,char *sValue)
  {
    if(strcmp(sType, "power") != 0)
      return false;
    
    const char *equals = strchr(sValue, '=');
    if(equals == NULL) {
      // No '=' character found!
      return false;
    }
    const int value = atoi(equals + 1);
    
    if(strncmp(sValue, "IDLE", 4) == 0) {
      powerTable[Component::IDLE] = value;
    } else if(strncmp(sValue, "RUNNING", 7) == 0) {
      powerTable[Component::RUNNING] = value;
    } else {
      // Not a supported power mode
      return false;
    }
    
    return true;
  }

  /**
   *
   */
  void Component::processAndForwardParameter(char *sType,char *sValue){
    if(processParameter(sType, sValue))
      return;
    scheduler->setProperty(sType,sValue);
  }
  
  void Component::processAndForwardAttribute(Attribute& fr_Attributes){
    scheduler->setAttribute(fr_Attributes);
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
  void Component::informAboutMapping(string module){
#ifndef NO_VCD_TRACES
    Tracing *newsignal = new Tracing();
    trace_map_by_name.insert(std::pair<string,Tracing* >(module, newsignal));
    sc_trace(this->traceFile, *newsignal->traceSignal, module.c_str());
#endif //NO_VCD_TRACES

  }

  /**
   *
   */
  void Component::compute(ProcessControlBlock* actualTask){ 

    DBG_OUT("Component::compute ( " << actualTask->getName()
            << " ) at time: " << sc_time_stamp()
            << endl);

    // reset the execution delay
    actualTask->
      setRemainingDelay(actualTask->getFuncDelay(this->getComponentId(),
                                                 actualTask->getFunctionId()));
    actualTask->
      setDelay(actualTask->getFuncDelay(this->getComponentId(),
                                        actualTask->getFunctionId()));
    actualTask->
      setLatency(actualTask->getFuncLatency(this->getComponentId(),
                                            actualTask->getFunctionId()));
#ifdef VPC_DEBUG
    cerr << "Using " << actualTask->getRemainingDelay()
         << " as delay for function " << actualTask->getFuncName() << "!"
         << endl;
    cerr << "And " << actualTask->getLatency() << " as latency for function "
         << actualTask->getFuncName() << "!" << endl;
#endif // VPC_DEBUG
    
#ifndef NO_VCD_TRACES
    {
      std::map<string, Tracing* >::iterator iter
        = trace_map_by_name.find(actualTask->getName());
      if( iter != trace_map_by_name.end() ){
        actualTask->setTraceSignal(iter->second);
      }
    }
#endif //NO_VCD_TRACES


    //int process=actualTask->iid;

    // register start of task
    actualTask->setState(starting);
    Director::getInstance().checkConstraints();
    actualTask->setState(aktiv);

    //store added task
    newTasks.push_back(actualTask);

    //awake scheduler thread
    notify_scheduler_thread.notify();
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
        sc_time waitFor	= front.time-sc_time_stamp();
        assert(front.time >= sc_time_stamp());
        //cerr << "Pipeline> Wait till " << front.time
        //<< " (" << waitFor << ") at: " << sc_time_stamp() << endl;
        wait( waitFor, remainingPipelineStages_WakeUp );

        //DISABLED: if a task completes at same time
        // (but not necessarily in same delta cycle)
        //          then this task is completed nonetheless
        //if( !this->isActiv() ) continue;

        sc_time rest = front.time-sc_time_stamp();
        assert(rest >= SC_ZERO_TIME);
        if(rest > SC_ZERO_TIME){
          //cerr << "------------------------------" << endl;
        }else{
          assert(rest == SC_ZERO_TIME);
          //cerr << "Ready! releasing task (" <<  front.time <<") at: "
          //<< sc_time_stamp() << endl;


          // Latency over -> remove Task
          this->notifyParentController(front.pcb);

          //wait(SC_ZERO_TIME);
          pqueue.pop();
        }
      }

    }
  }

  /**
   *
   */
  void Component::moveToRemainingPipelineStages(ProcessControlBlock* task){
    sc_time now                 = sc_time_stamp();
    sc_time restOfLatency       = task->getLatency() - task->getDelay();
    sc_time end                 = now + restOfLatency;
    if(end <= now){
      //early exit if (Latency-DII) <= 0
      //std::cerr << "Early exit: " << task->getName() << std::endl;
      this->notifyParentController(task);
      return;
    }
    timePcbPair pair;
    pair.time = end;
    pair.pcb  = task;
    //std::cerr << "Rest of pipeline added: " << task->getName()
    //<< " (EndTime: " << pair.time << ") " << std::endl;
    pqueue.push(pair);
    remainingPipelineStages_WakeUp.notify();
  }

  void Component::setComponentState(const ComponentState &state)
  {
    this->setPowerConsumption(powerTable[state]);
  }
} //namespace SystemC_VPC
