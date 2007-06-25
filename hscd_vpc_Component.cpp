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
#include <hscd_vpc_RoundRobinScheduler.h>
#include <hscd_vpc_PriorityScheduler.h>
#include <hscd_vpc_RateMonotonicScheduler.h>
#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_Director.h>

#include <float.h>

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
    while(1){

      //determine the time slice for next scheduling descission and wait for
      bool hasTimeSlice= scheduler->getSchedulerTimeSlice( timeslice,
                                                           readyTasks,
                                                           runningTasks );
      startTime = sc_time_stamp();
      if(!newTaskDuringOverhead && this->isActiv()){ 
        if(runningTasks.size()<=0){                    // no running task
          if(hasTimeSlice){                           
            wait( timeslice - (*overhead),
                  notify_scheduler_thread | notify_deallocate ); 
          }else{
            wait( notify_scheduler_thread | notify_deallocate );
          }
        }else{                                        // a task allready runs
          if(hasTimeSlice && (timeslice - (*overhead)) < actualRemainingDelay){
            wait( timeslice - (*overhead),
                  notify_scheduler_thread | notify_deallocate );
          }else{
            wait( actualRemainingDelay,
                  notify_scheduler_thread | notify_deallocate );
          }
          sc_time runTime=sc_time_stamp()-startTime;
          assert(runTime.value()>=0);
          actualRemainingDelay-=runTime;

          assert(actualRemainingDelay.value()>=0);

#ifdef VPC_DEBUG
          std::cerr << VPC_RED("Component " << this->basename()
                    << "> actualRemainingDelay= "
                    << actualRemainingDelay.value() << " for iid="
                    << actualRunningIID << " at: " << sc_simulation_time())
                    << std::endl;
#endif //VPC_DEBUG

          if(actualRemainingDelay.value()==0){
            // all execution time simulated -> BLOCK running task.
            ProcessControlBlock *task=runningTasks[actualRunningIID];

            task->setState(ending);
            Director::getInstance().checkConstraints();
            task->setState(inaktiv);

#ifdef VPC_DEBUG
            cerr << this->basename() << " IID: " << actualRunningIID<< " > ";
            cerr << this->basename() << " removed Task: " << task->getName()
                 << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

            //notify(*(task->blockEvent));
            scheduler->removedTask(task);
#ifndef NO_VCD_TRACES
            if(task->getTraceSignal()!=0)
              task->getTraceSignal()->value(S_BLOCKED);     
#endif //NO_VCD_TRACES
            runningTasks.erase(actualRunningIID);

            // Removed: this will erase the task from VPC, but with pipelining
            // support the erasing should occur later 
            //this->notifyParentController(task);
            
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

      // before making any scheduling decision:
      // check if component is deallocated
      // and remain in this state as long as component is deactivated
      while(! this->isActiv()){

#ifdef VPC_DEBUG
        std::cerr << VPC_RED( this->basename()  << " deactivated at ")
                  << sc_simulation_time() << std::endl;    
#endif // VPC_DEBUG

        //check if deallocation is with kill flag
        if(this->killed){
          this->killAllTasks();
        }else{

#ifndef NO_VCD_TRACES
          this->setTraceSignalReadyTasks(S_SUSPENDED);

          if( this->runningTasks.size() > 0
              && runningTasks[actualRunningIID]->getTraceSignal() != NULL
              ){
            runningTasks[actualRunningIID]->getTraceSignal()
              ->value(S_SUSPENDED);
          }     
#endif //NO_VCD_TRACES

        }

        // wait until allocate is signalled or another deallocate happend
        this->wait(this->notify_deallocate | this->notify_allocate);

        // if component still inactiv just jump to beginning of loop
        if(!this->isActiv())  continue;

#ifndef NO_VCD_TRACES
        this->setTraceSignalReadyTasks(S_READY);

        if( this->runningTasks.size() > 0
            && runningTasks[actualRunningIID]->getTraceSignal() != NULL
            ){
          runningTasks[actualRunningIID]->getTraceSignal()->value(S_RUNNING);
        }     
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
        std::cerr << VPC_RED( this->basename()  << " reactivated at ")
                  << sc_simulation_time() << std::endl;    
#endif // VPC_DEBUG

      }

      //look for new tasks (they called compute)
      while(newTasks.size()>0){
        ProcessControlBlock *newTask;
        newTask=newTasks[0];
        newTasks.pop_front();
#ifdef VPC_DEBUG
        cerr << this->basename() << " received new Task: "
             << newTask->getName() << " at: " << sc_simulation_time() << endl;
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
        while( (sc_time_stamp() < timestamp + (*overhead))
            && this->isActiv() ){ 

          wait( (timestamp+(*overhead))-sc_time_stamp(),
                notify_scheduler_thread | notify_deallocate);

        }

        /************************/
        /*  EXTENSION SECTION   */
        /************************/

        if(! this->isActiv()){
          // just jump to begining of loop to process deallocation
          continue;
        }

        /**************************/
        /*  END OF EXTENSION      */
        /**************************/

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
#ifndef NO_VCD_TRACES
        if(runningTasks[taskToAssign]->getTraceSignal()!=0)
          runningTasks[taskToAssign]->getTraceSignal()->value(S_RUNNING);     
#endif //NO_VCD_TRACES
      }
    }

  }

  /**
   *
   */
  void Component::processAndForwardParameter(char *sType,char *sValue){
    scheduler->setProperty(sType,sValue);
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

    }else{
      //    cerr << "Scheduler: "<< STR_FIRSTCOMEFIRSTSERVE << endl;
      scheduler=new FCFSScheduler();
    }
  }

  /**
   *
   */
  void Component::_compute( const char *name,
                            const char *funcname,
                            VPC_Event *end) { 
    ProcessControlBlock  *actualTask =
      Director::getInstance().getProcessControlBlock(name);

#ifdef VPC_DEBUG
    cout << flush;
    cerr << VPC_RED("Component::compute( ") << VPC_WHITE(actualTask->getName())
         << VPC_RED(" , ") << VPC_WHITE(actualTask->getFuncName()) 
         << VPC_RED(" ) at time: " << sc_simulation_time()) << endl
      ;
#endif

    // reset the execution delay
    actualTask->
      setRemainingDelay(actualTask->getFuncDelay(this->basename(),
                                                 actualTask->getFuncName()));
    actualTask->
      setDelay(actualTask->getFuncDelay(this->basename(),
                                        actualTask->getFuncName()));
    actualTask->
      setLatency(actualTask->getFuncLatency(this->basename(),
                                            actualTask->getFuncName()));
#ifdef VPC_DEBUG
    cerr << "Using " << actualTask->getRemainingDelay()
         << " as delay for function " << actualTask->getFuncName() << "!"
         << endl;
    cerr << "And using " << actualTask->getLatency()
         << " as latency for function " << actualTask->getFuncName() << "!"
         << endl;
#endif // VPC_DEBUG
    
    if( actualTask->getBlockEvent().dii == NULL ){
      // active mode -> returns if simulated delay time has expired
      // (blocking compute call)
      actualTask->setBlockEvent(EventPair(new VPC_Event(), new VPC_Event()));
      compute(actualTask);
      CoSupport::SystemC::wait(*(actualTask->getBlockEvent().dii));
      delete actualTask->getBlockEvent().dii;
      delete actualTask->getBlockEvent().latency;
      actualTask->setBlockEvent(EventPair());
      // return
    } else {
      // passive mode -> return immediatly (no blocking)
      compute(actualTask);
    }

    return;
  }

  /**
   *
   */
  void Component::_compute( const char *name, VPC_Event *end) { 
#ifdef VPC_DEBUG
    cout << flush;
    cerr << VPC_RED("Component::compute( ") << VPC_WHITE(name)
         << VPC_RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

    _compute(name,"",end);
  }

  /**
   *
   */
  void Component::informAboutMapping(string module){
#ifndef NO_VCD_TRACES
    Tracing *newsignal = new Tracing();
    trace_map_by_name.insert(pair<string,Tracing* >(module, newsignal));
    sc_trace(this->traceFile, *newsignal->traceSignal, module.c_str());
#endif //NO_VCD_TRACES

  }

  /************************/
  /*  EXTENSION SECTION   */
  /************************/

  /**
   * \brief Implementation of Component::deallocate
   */
  void Component::deallocate(bool kill){

    // deallocate only activ component
    // or additional deallocate if component not killed
    if(this->isActiv() || (kill = true && this->killed == false)){
      this->setActiv(false);
      this->killed = kill;
      this->notify_deallocate.notify();
      interuptPipeline(kill);
      //wait(SC_ZERO_TIME);
    }

  }

  /**
   * \brief Implementation of Component::allocate
   */
  void Component::allocate(){

    // allocate only deallocated component
    if(!this->isActiv()){
      this->setActiv(true);
      this->killed = false;
      this->notify_allocate.notify();
      resumePipeline();
      //wait(SC_ZERO_TIME);
    }

  }

  /**
   *
   */
  void Component::compute(ProcessControlBlock* actualTask){ 

#ifdef VPC_DEBUG
    cout << flush;
    cerr << VPC_RED("Component::compute( ") << VPC_WHITE(actualTask->getName())
         << VPC_RED(" , ") << VPC_WHITE(actualTask->getFuncName())
         << VPC_RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

    // reset the execution delay
    actualTask->
      setRemainingDelay(actualTask->getFuncDelay(this->basename(),
                                                 actualTask->getFuncName()));
    actualTask->
      setDelay(actualTask->getFuncDelay(this->basename(),
                                        actualTask->getFuncName()));
    actualTask->
      setLatency(actualTask->getFuncLatency(this->basename(),
                                            actualTask->getFuncName()));
#ifdef VPC_DEBUG
    cerr << "Using " << actualTask->getRemainingDelay()
         << " as delay for function " << actualTask->getFuncName() << "!"
         << endl;
    cerr << "And " << actualTask->getLatency() << " as latency for function "
         << actualTask->getFuncName() << "!" << endl;
#endif // VPC_DEBUG
    
    //*********************************************************
    // * SECTION FROM OLD METHOD COMPUTE(P_STRUCT ACTUAL_TASK)
    // *********************************************************/


#ifndef NO_VCD_TRACES
    {
      map<string, Tracing* >::iterator iter
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
    notify(notify_scheduler_thread);
    //wait(SC_ZERO_TIME);

    ////////////////////////////////////////////////
    //events.push_back(actualTask->blockEvent);//
    //notify(notify_scheduler_thread);            //
    //return;                                     //
    ////////////////////////////////////////////////
  }

  void Component::killAllTasks(){

    // kill all running tasks
    std::map<int,ProcessControlBlock* >::iterator iter;
    for(iter = this->runningTasks.begin();
        iter != this->runningTasks.end();
        ++iter){

      iter->second->setState(activation_state(aborted));

#ifdef VPC_DEBUG
      cerr << this->basename() << " > ";
      cerr << this->basename() << " killed Task: " << iter->second->getName()
           << " activation state set to "<< iter->second->getState() << " at: "
           << sc_simulation_time() << endl;
#endif // VPCDEBUG

      scheduler->removedTask(iter->second);

      // reset actualTask  
      iter->second->setDelay(SC_ZERO_TIME);
      iter->second->setRemainingDelay(SC_ZERO_TIME);

#ifdef VPC_DEBUG
      cerr << this->basename() << " IID: " <<  iter->second->getInstanceId()
           << " > ";
      cerr << this->basename() << " killed Task: " << iter->second->getName()
           << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

#ifndef NO_VCD_TRACES
      if(iter->second->getTraceSignal() != 0){
        iter->second->getTraceSignal()->value(S_KILLED);
      }
#endif //NO_VCD_TRACES

      this->parentControlUnit->signalProcessEvent(iter->second);
    }

    //clear all entries
    this->runningTasks.clear();

    // kill all ready tasks
    for(iter = this->readyTasks.begin();
        iter != this->readyTasks.end();
        ++iter){
      iter->second->setState(activation_state(aborted));

#ifdef VPC_DEBUG
      cerr << this->basename() << " > ";
      cerr << this->basename() << " killed Task: " << iter->second->getName()
           << " activation state set to "<< iter->second->getState() << " at: "
           << sc_simulation_time() << endl;
#endif // VPCDEBUG

      scheduler->removedTask(iter->second);

      // reset actualTask  
      iter->second->setDelay(SC_ZERO_TIME);
      iter->second->setRemainingDelay(SC_ZERO_TIME);

#ifdef VPC_DEBUG
      cerr << this->basename() << " IID: " <<  iter->second->getInstanceId()
           << " > ";
      cerr << this->basename() << " killed Task: " << iter->second->getName()
           << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

#ifndef NO_VCD_TRACES
      if(iter->second->getTraceSignal() != 0){
        iter->second->getTraceSignal()->value(S_KILLED);
      }
#endif //NO_VCD_TRACES

      this->parentControlUnit->signalProcessEvent(iter->second);
    }

    this->readyTasks.clear();

    // finally check if also new task have to be removed
    while(newTasks.size()>0){

      ProcessControlBlock *newTask;
      newTask = newTasks.front();
      newTasks.pop_front();
      newTask->setState(activation_state(aborted));

#ifdef VPC_DEBUG
      cerr << this->basename() << " > ";
      cerr << this->basename() << " killed Task: " << newTask->getName()
           << " activation state set to "<< newTask->getState() << " at: "
           << sc_simulation_time() << endl;
#endif // VPCDEBUG

      //reset actualTask
      newTask->setDelay(SC_ZERO_TIME);
      newTask->setRemainingDelay(SC_ZERO_TIME);

      this->parentControlUnit->signalProcessEvent(newTask);

#ifndef NO_VCD_TRACES
      if(newTask->getTraceSignal() !=0 ){
        newTask->getTraceSignal()->value(S_KILLED); 
      }
#endif //NO_VCD_TRACES

    }

  }

  void Component::setTraceSignalReadyTasks(trace_value value){

    std::map<int, ProcessControlBlock* >::iterator iter;
    for(iter = this->readyTasks.begin();
        iter != this->readyTasks.end();
        ++iter){

      if(iter->second->getTraceSignal() !=0 ){
        iter->second->getTraceSignal()->value(value);
      }

    }

  }

  /**************************/
  /*  END OF EXTENSION      */
  /**************************/


  /**
   *
   */
  void Component::remainingPipelineStages(){
    while(1){
      if( (!this->isActiv()) || (pqueue.size() == 0) ){
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


          // Latenzy over -> remove Task
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

  /**
   *
   */
  void Component::interuptPipeline(bool kill){

    priority_queue<timePcbPair, vector<timePcbPair>,timeCompare> temp;
    if(kill){
      while(pqueue.size()>0){
        timePcbPair top = pqueue.top();
        top.pcb->setState(activation_state(aborted));
        scheduler->removedTask(top.pcb);
#ifndef NO_VCD_TRACES
        if(top.pcb->getTraceSignal()!=0)
          top.pcb->getTraceSignal()->value(S_KILLED);
#endif //NO_VCD_TRACES
        this->parentControlUnit->signalProcessEvent(top.pcb);

        pqueue.pop();
      }

      assert(pqueue.empty());
    }else{
      //making absolute timing to relative timings (deduct actual time stamp)
      while(pqueue.size()>0){
        timePcbPair top = pqueue.top();
        pqueue.pop();
        top.time -= sc_time_stamp();
        temp.push(top);
      }
      pqueue = temp;
    }
    remainingPipelineStages_WakeUp.notify();
  }

  /**
   *
   */
  void Component::resumePipeline(){
    priority_queue<timePcbPair, vector<timePcbPair>,timeCompare> temp;
    //making relative timings to absolute timings (add actual time stamp)
    while(pqueue.size()>0){
      timePcbPair top = pqueue.top();
      top.time += sc_time_stamp();
      temp.push(top);
      pqueue.pop();
    }
    pqueue = temp;

    remainingPipelineStages_WakeUp.notify();
  }

} //namespace SystemC_VPC
