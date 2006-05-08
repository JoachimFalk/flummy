/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Component.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#include <hscd_vpc_Component.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_FCFSScheduler.h>
#include <hscd_vpc_RoundRobinScheduler.h>
#include <hscd_vpc_PriorityScheduler.h>
#include <hscd_vpc_RateMonotonicScheduler.h>
#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_Director.h>


//#include <cosupport/systemc_support.hpp>

#include <float.h>

namespace SystemC_VPC{
 
  /**
   *
   */
  void Component::schedule_thread(){
    /////////////////////////////////////
    // while(1){
    //  wait(notify_scheduler_thread);
    //   do{
    //     wait(2,SC_NS);
    //     notify(*(events.front()));
    //     events.pop_front();
    //   }while(events.size()>0);
    // }
    /////////////////////////////////////
    sc_time timeslice;
    sc_time actualRemainingDelay;
    sc_time *overhead;
    int actualRunningPID;
    bool newTaskDuringOverhead=false;
    //wait(SC_ZERO_TIME);
    while(1){

      //determine the time slice for next scheduling descission and wait for
      bool hasTimeSlice= scheduler->getSchedulerTimeSlice(timeslice, readyTasks,runningTasks);
      startTime = sc_time_stamp();
      if(!newTaskDuringOverhead && this->isActiv()){ 
        if(runningTasks.size()<=0){                    // no running task
          if(hasTimeSlice){                           
            wait(timeslice - (*overhead), notify_scheduler_thread | notify_preempt); 
          }else{
            wait(notify_scheduler_thread | notify_preempt);
          }
        }else{                                        // a task allready runs
          if(hasTimeSlice && (timeslice - (*overhead)) < actualRemainingDelay){ 
            wait(timeslice - (*overhead), notify_scheduler_thread | notify_preempt);
          }else{
            wait(actualRemainingDelay, notify_scheduler_thread | notify_preempt);
          }
          sc_time runTime=sc_time_stamp()-startTime;
          assert(runTime.value()>=0);
          actualRemainingDelay-=runTime;
      
          assert(actualRemainingDelay.value()>=0);
    
#ifdef VPC_DEBUG
          std::cerr << RED("Component " << this->basename() << "> actualRemainingDelay= " << actualRemainingDelay.value()
                    << " for pid=" << actualRunningPID << " at: " << sc_simulation_time()) << std::endl;
#endif //VPC_DEBUG
          
          if(actualRemainingDelay.value()==0){
            // all execution time simulated -> BLOCK running task.
            ProcessControlBlock *task=runningTasks[actualRunningPID];

            task->setState(ending);
            Director::getInstance().checkConstraints();
            task->setState(inaktiv);

#ifdef VPC_DEBUG
            cerr << this->basename() << " PID: " << actualRunningPID<< " > ";
            cerr << this->basename() << " removed Task: " << task->getName() << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
            
            //notify(*(task->blockEvent));
            scheduler->removedTask(task);
#ifndef NO_VCD_TRACES
            if(task->getTraceSignal()!=0) *(task->getTraceSignal())=S_BLOCKED;     
#endif //NO_VCD_TRACES
            runningTasks.erase(actualRunningPID);
      
            this->notifyParentController(task);
      
            //wait(SC_ZERO_TIME);
          }else{
       
            // store remainingDela within ProcessControlBlock
            runningTasks[actualRunningPID]->setRemainingDelay(actualRemainingDelay.to_default_time_units());
      
          }
        }
      }else{
        newTaskDuringOverhead=false;
      }

      // before making any scheduling decision check if component is preempted
      if(! this->isActiv()){

#ifdef VPC_DEBUG
        std::cerr << RED( this->basename()  << " deactivated at ") << sc_simulation_time() << std::endl;    
#endif // VPC_DEBUG
    
        //check if preemption is with kill flag
        if(this->killed){
          this->killAllTasks();
        }else{

#ifndef NO_VCD_TRACES
          this->setTraceSignalReadyTasks(S_SUSPENDED);
        
          if(this->runningTasks.size() > 0 && runningTasks[actualRunningPID]->getTraceSignal() != NULL){
            *(runningTasks[actualRunningPID]->getTraceSignal())=S_SUSPENDED;
          }     
#endif //NO_VCD_TRACES
      
        }
    
        //this->wait(SC_ZERO_TIME);
        // wait until resume is signalled
        this->wait(notify_resume);

#ifndef NO_VCD_TRACES
        this->setTraceSignalReadyTasks(S_READY);
    
        if(this->runningTasks.size() > 0 && runningTasks[actualRunningPID]->getTraceSignal() != NULL){
           *(runningTasks[actualRunningPID]->getTraceSignal())=S_RUNNING;
        }     
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
        std::cerr << RED( this->basename()  << " reactivated at ") << sc_simulation_time() << std::endl;    
#endif // VPC_DEBUG
    
      }

      //look for new tasks (they called compute)
      while(newTasks.size()>0){
        ProcessControlBlock *newTask;
        newTask=newTasks[0];
        newTasks.pop_front();
#ifdef VPC_DEBUG
        cerr << this->basename() << " received new Task: " << newTask->getName() << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
#ifndef NO_VCD_TRACES
        *(newTask->getTraceSignal())=S_READY;     
#endif //NO_VCD_TRACES
        //insert new task in read list
        assert(readyTasks.find(newTask->getPID())   == readyTasks.end()   /* A task can call compute only one time! */);
        assert(runningTasks.find(newTask->getPID()) == runningTasks.end() /* A task can call compute only one time! */);
        
        readyTasks[newTask->getPID()]=newTask;
        scheduler->addedNewTask(newTask);
      }

     
      int taskToResign,taskToAssign;
      scheduling_decision decision=
      scheduler->schedulingDecision(taskToResign, taskToAssign, readyTasks, runningTasks);
  

      //resign task
      if(decision==RESIGNED || decision==PREEMPT){
        readyTasks[taskToResign]=runningTasks[taskToResign];
        runningTasks.erase(taskToResign);
        actualRunningPID=-1;
        readyTasks[taskToResign]->setRemainingDelay(actualRemainingDelay.to_default_time_units());
#ifndef NO_VCD_TRACES
        if(readyTasks[taskToResign]->getTraceSignal()!=0) *(readyTasks[taskToResign]->getTraceSignal())=S_READY;     
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
          
          wait((timestamp+(*overhead))-sc_time_stamp(), notify_scheduler_thread | notify_preempt);
      
        }

        /************************/
        /*  EXTENSION SECTION   */
        /************************/
  
        if(! this->isActiv()){
          // just jump to begining of loop to process preemption
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
        actualRunningPID=taskToAssign;
#ifdef VPC_DEBUG
        cerr << "PID: " << taskToAssign;
        cerr << "> remaining delay for " << runningTasks[taskToAssign]->getName();
#endif // VPCDEBUG
        actualRemainingDelay=sc_time(runningTasks[taskToAssign]->getRemainingDelay(),SC_NS);
#ifdef VPC_DEBUG
        cerr<< " is " << runningTasks[taskToAssign]->getRemainingDelay() << endl;
#endif // VPCDEBUG
#ifndef NO_VCD_TRACES
        if(runningTasks[taskToAssign]->getTraceSignal()!=0) *(runningTasks[taskToAssign]->getTraceSignal())=S_RUNNING;     
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
    if(0==strncmp(schedulername,STR_ROUNDROBIN,strlen(STR_ROUNDROBIN)) || 0==strncmp(schedulername,STR_RR,strlen(STR_RR))){
      scheduler=new RoundRobinScheduler((const char*)schedulername);
    }else if(0==strncmp(schedulername,STR_PRIORITYSCHEDULER,strlen(STR_PRIORITYSCHEDULER)) || 0==strncmp(schedulername,STR_PS,strlen(STR_PS))){
      scheduler=new PriorityScheduler((const char*)schedulername);
    }else if(0==strncmp(schedulername,STR_RATEMONOTONIC,strlen(STR_RATEMONOTONIC)) || 0==strncmp(schedulername,STR_RM,strlen(STR_RM))){
      scheduler=new RateMonotonicScheduler((const char*)schedulername);
    }else if(0==strncmp(schedulername,STR_FIRSTCOMEFIRSTSERVE,strlen(STR_FIRSTCOMEFIRSTSERVE)) || 0==strncmp(schedulername,STR_FCFS,strlen(STR_FCFS))){
      scheduler=new FCFSScheduler();
    }else{
      //    cerr << "Scheduler: "<< STR_FIRSTCOMEFIRSTSERVE << endl;
      scheduler=new FCFSScheduler();
    }
  }

  /**
   *
   */
  void Component::compute( const char *name, const char *funcname, VPC_Event *end) { 
    ProcessControlBlock  *actualTask = Director::getInstance().getProcessControlBlock(name);

#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component " << this->basename() << "> compute( ") <<WHITE(actualTask->getName())<<RED(" , ")<<WHITE(actualTask->getFuncName())<<RED(" ) at time: " << sc_simulation_time()) << endl
      ;
#endif

#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *trace_signal=0;
    if(1==trace_map_by_name.count(actualTask->getName())){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->getName());
      trace_signal=(iter->second);
    }
#endif //NO_VCD_TRACES

    /*
     *  REMOVED SECTION AS SETTING UP DELAY IS DONE BY BINDER INSTANCES NOW
     * 
#ifdef VPC_DEBUG
    if(!actualTask->hasDelay(this->basename(), actualTask->getFuncName()))
      cerr << RED("VPC_LOGICAL_ERROR> ") << YELLOW("having \"functionDelays\" in general, but no delay for this function (")<< actualTask->getFuncName() <<YELLOW(")!")
        << endl;

    std::cerr << "Component> Check if special delay exist for "<< actualTask->getFuncName() << " on " << this->basename() << ": " << (actualTask->hasDelay(this->basename(), actualTask->getFuncName())) << std::endl;
#endif // VPC_DEBUG

    // reset the execution delay
    if( actualTask->hasDelay(this->basename(), actualTask->getFuncName())){
      // function specific delay
      actualTask->setRemainingDelay(actualTask->getFuncDelay(this->basename(), actualTask->getFuncName()));
#ifdef VPC_DEBUG
      cerr << "Using " << actualTask->getRemainingDelay() << " as delay for function " << actualTask->getFuncName() << "!" << endl;
#endif // VPC_DEBUG
    } else {
      // general delay for actor
      actualTask->setRemainingDelay(actualTask->getFuncDelay(this->basename()));
#ifdef VPC_DEBUG
      cerr << "Using standard delay " << actualTask->getRemainingDelay() << " as delay for function " << actualTask->getFuncName() << "!" << endl;
#endif // VPC_DEBUG
    }
    */

    if( actualTask->getBlockEvent() == NULL ){
      // active mode -> returns if simulated delay time has expired (blocking compute call)
      actualTask->setBlockEvent(new VPC_Event());
      compute(actualTask);
      CoSupport::SystemC::wait(*(actualTask->getBlockEvent()));
      delete actualTask->getBlockEvent();
      actualTask->setBlockEvent(NULL);
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
  void Component::compute( const char *name, VPC_Event *end) { 
#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component::compute( ") << WHITE(name)<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

    compute(name,"",end);
  }

  /**
   *
   */
  void Component::informAboutMapping(string module){
#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *newsignal=new sc_signal<trace_value>();
    trace_map_by_name.insert(pair<string,sc_signal<trace_value>*>(module,newsignal));
    sc_trace(this->traceFile,*newsignal,module.c_str());
#endif //NO_VCD_TRACES

  }

  /************************/
  /*  EXTENSION SECTION   */
  /************************/
  
  /**
   * \brief Implementation of Component::preempt
   */
  void Component::preempt(bool kill){

    // preempt only activ component
    if(this->isActiv()){
      this->setActiv(false);
      this->killed = kill;
      this->notify_preempt.notify();
      //wait(SC_ZERO_TIME);
    }
   
  }

  /**
   * \brief Implementation of Component::resume
   */
  void Component::resume(){
      
    // resume only preempted component
    if(!this->isActiv()){
      this->setActiv(true);
      this->killed = false;
      this->notify_resume.notify();
      //wait(SC_ZERO_TIME);
    }

  }

  /**
   *
   */
  void Component::compute(ProcessControlBlock* actualTask){ 

#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component " << this->basename() << "> compute( ") <<WHITE(actualTask->getName())<<RED(" , ")<<WHITE(actualTask->getFuncName())<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *trace_signal=0;
    if(1==trace_map_by_name.count(actualTask->getName())){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->getName());
      trace_signal=(iter->second);
    }
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
    if(!actualTask->hasDelay(this->basename(), actualTask->getFuncName()))
      cerr << RED("VPC_LOGICAL_ERROR> ") << YELLOW("having \"functionDelays\" in general, but no delay for this function (")<< actualTask->getFuncName() <<YELLOW(")!") << endl;

    std::cerr << "Component> Check if special delay exist for "<< actualTask->getFuncName() << " on " << this->basename() << ": " << (actualTask->hasDelay(basename())) << std::endl;
#endif // VPC_DEBUG
  
    /*
     * REMOVED SECTION AS SETTING UP DELAY IS DONE BY BINDER INSTANCES NOW
     * 
    // reset the execution delay
    if( actualTask->hasDelay(this->basename(), actualTask->getFuncName())){
      // function specific delay
      actualTask->setDelay(actualTask->getFuncDelay(this->basename(), actualTask->getFuncName()));
      actualTask->setRemainingDelay(actualTask->getDelay());
#ifdef VPC_DEBUG
      cerr << "Using " << actualTask->getRemainingDelay() << " as delay for function " << actualTask->getFuncName() << "!" << endl;
#endif // VPC_DEBUG
    } else {
      // general delay for actor
      actualTask->setDelay(actualTask->getFuncDelay(this->basename()));
      actualTask->setRemainingDelay(actualTask->getDelay());
#ifdef VPC_DEBUG
      cerr << "Using standard delay " << actualTask->getRemainingDelay() << " as delay for function " << actualTask->getFuncName() << "!" << endl;
#endif // VPC_DEBUG  
    }
  */
    //*********************************************************
    // * SECTION FROM OLD METHOD COMPUTE(P_STRUCT ACTUAL_TASK)
    // *********************************************************/
    
  
  #ifndef NO_VCD_TRACES
    if(1==trace_map_by_name.count(actualTask->getName())){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->getName());
      actualTask->setTraceSignal(iter->second);
    }
  #endif //NO_VCD_TRACES


    //int process=actualTask->pid;

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
    for(iter = this->runningTasks.begin(); iter != this->runningTasks.end(); iter++){
      
      iter->second->setState(activation_state(aborted));
      
#ifdef VPC_DEBUG
        cerr << this->basename() << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->getName()
        << " activation state set to "<< iter->second->getState() << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

      scheduler->removedTask(iter->second);
      
      // reset actualTask  
      iter->second->setDelay(0);
      iter->second->setRemainingDelay(0);
      
#ifdef VPC_DEBUG
        cerr << this->basename() << " PID: " <<  iter->second->getPID() << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->getName() << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

#ifndef NO_VCD_TRACES
        if(iter->second->getTraceSignal() != 0){
          *(iter->second->getTraceSignal()) = S_KILLED;
        }     
#endif //NO_VCD_TRACES
    
      this->parentControlUnit->signalTaskEvent(iter->second);

    }
    
    //clear all entries
    this->runningTasks.clear();
    
    // kill all ready tasks
    for(iter = this->readyTasks.begin(); iter != this->readyTasks.end(); iter++){
      iter->second->setState(activation_state(aborted));

#ifdef VPC_DEBUG
        cerr << this->basename() << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->getName()
        << " activation state set to "<< iter->second->getState() << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
      
      scheduler->removedTask(iter->second);
      
      // reset actualTask  
      iter->second->setDelay(0);
      iter->second->setRemainingDelay(0);
      
#ifdef VPC_DEBUG
        cerr << this->basename() << " PID: " <<  iter->second->getPID() << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->getName() << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

#ifndef NO_VCD_TRACES
        if(iter->second->getTraceSignal() != 0){
          *(iter->second->getTraceSignal()) = S_KILLED;
        }     
#endif //NO_VCD_TRACES

      this->parentControlUnit->signalTaskEvent(iter->second);
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
        << " activation state set to "<< newTask->getState() << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
 
      //reset actualTask
      newTask->setDelay(0);
      newTask->setRemainingDelay(0);
      
      this->parentControlUnit->signalTaskEvent(newTask);
      
#ifndef NO_VCD_TRACES
      if(newTask->getTraceSignal() != 0){
        *(newTask->getTraceSignal())=S_KILLED; 
      }    
#endif //NO_VCD_TRACES
                
      }
    
    //wait(SC_ZERO_TIME);    
  }
  
  void Component::setTraceSignalReadyTasks(trace_value value){
      
      std::map<int, ProcessControlBlock* >::iterator iter;
      for(iter = this->readyTasks.begin(); iter != this->readyTasks.end(); iter++){

        if(iter->second->getTraceSignal() != 0){
          *(iter->second->getTraceSignal()) = value;
        }     

      }
      
  }
  
  /**************************/
  /*  END OF EXTENSION      */
  /**************************/

} //namespace SystemC_VPC
