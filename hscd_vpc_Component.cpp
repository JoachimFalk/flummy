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
            p_struct *task=runningTasks[actualRunningPID];

            task->state=ending;
            Director::getInstance().checkConstraints();
            task->state=inaktiv;

#ifdef VPC_DEBUG
            cerr << this->basename() << " PID: " << actualRunningPID<< " > ";
            cerr << this->basename() << " removed Task: " << task->name << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
            
            //notify(*(task->blockEvent));
            scheduler->removedTask(task);
#ifndef NO_VCD_TRACES
            if(task->traceSignal!=0) *(task->traceSignal)=S_BLOCKED;     
#endif //NO_VCD_TRACES
            runningTasks.erase(actualRunningPID);
      
            this->notifyParentController(task);
      
            //wait(SC_ZERO_TIME);
          }else{
       
            // store remainingDela within p_struct
            runningTasks[actualRunningPID]->remainingDelay = actualRemainingDelay.to_default_time_units();
      
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
        
          if(this->runningTasks.size() > 0 && runningTasks[actualRunningPID]->traceSignal != NULL){
            *(runningTasks[actualRunningPID]->traceSignal)=S_SUSPENDED;
          }     
#endif //NO_VCD_TRACES
      
        }
    
        //this->wait(SC_ZERO_TIME);
        // wait until resume is signalled
        this->wait(notify_resume);

#ifndef NO_VCD_TRACES
        this->setTraceSignalReadyTasks(S_READY);
    
        if(this->runningTasks.size() > 0 && runningTasks[actualRunningPID]->traceSignal != NULL){
           *(runningTasks[actualRunningPID]->traceSignal)=S_RUNNING;
        }     
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
        std::cerr << RED( this->basename()  << " reactivated at ") << sc_simulation_time() << std::endl;    
#endif // VPC_DEBUG
    
      }

      //look for new tasks (they called compute)
      while(newTasks.size()>0){
        p_struct *newTask;
        newTask=newTasks[0];
        newTasks.pop_front();
#ifdef VPC_DEBUG
        cerr << this->basename() << " received new Task: " << newTask->name << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
#ifndef NO_VCD_TRACES
        *(newTask->traceSignal)=S_READY;     
#endif //NO_VCD_TRACES
        //insert new task in read list
        assert(readyTasks.find(newTask->pid)   == readyTasks.end()   /* A task can call compute only one time! */);
        assert(runningTasks.find(newTask->pid) == runningTasks.end() /* A task can call compute only one time! */);
        
        readyTasks[newTask->pid]=newTask;
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
        readyTasks[taskToResign]->remainingDelay=actualRemainingDelay.to_default_time_units();
#ifndef NO_VCD_TRACES
        if(readyTasks[taskToResign]->traceSignal!=0) *(readyTasks[taskToResign]->traceSignal)=S_READY;     
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
        cerr << "> remaining delay for " << runningTasks[taskToAssign]->name;
#endif // VPCDEBUG
        actualRemainingDelay=sc_time(runningTasks[taskToAssign]->remainingDelay,SC_NS);
#ifdef VPC_DEBUG
        cerr<< " is " << runningTasks[taskToAssign]->remainingDelay << endl;
#endif // VPCDEBUG
#ifndef NO_VCD_TRACES
        if(runningTasks[taskToAssign]->traceSignal!=0) *(runningTasks[taskToAssign]->traceSignal)=S_RUNNING;     
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
    p_struct  *actualTask = Director::getInstance().getProcessControlBlock(name);
    actualTask->blockEvent=end;

#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component::compute( ") <<WHITE(name)<<RED(" , ")<<WHITE(funcname)<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *trace_signal=0;
    if(1==trace_map_by_name.count(actualTask->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->name);
      trace_signal=(iter->second);
    }
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
    if( (actualTask->compDelays[this->basename()]).size()>0 && ( (actualTask->compDelays[this->basename()]).count(funcname) == 0 ) )
      cerr << RED("VPC_LOGICAL_ERROR> ") << YELLOW("having \"functionDelays\" in general, but no delay for this function (")<< funcname <<YELLOW(")!") << endl;

      std::cerr << "Component> Check if special delay exist for "<< funcname << " on " << this->basename() << ": " << (actualTask->compDelays[this->basename()]).size() << std::endl;
#endif // VPC_DEBUG
  
    // reset the execution delay
    if( (actualTask->compDelays[this->basename()]).size()>0 && ( (actualTask->compDelays[this->basename()]).count(funcname) == 1 ) ){
    // function specific delay
      actualTask->remainingDelay = ((actualTask->compDelays[this->basename()])[funcname]);
#ifdef VPC_DEBUG
        cerr << "Using " << actualTask->remainingDelay << " as delay for function " << funcname << "!" << endl;
#endif // VPC_DEBUG
    } else {
      // general delay for actor
      actualTask->remainingDelay = actualTask->delay;
#ifdef VPC_DEBUG
      cerr << "Using standard delay " << actualTask->remainingDelay << " as delay for function " << funcname << "!" << endl;
#endif // VPC_DEBUG  
    }
  

    if( actualTask->blockEvent == NULL ){
      // active mode -> returns if simulated delay time has expired (blocking compute call)
      actualTask->blockEvent = new VPC_Event();
      compute(actualTask);
      CoSupport::SystemC::wait(*(actualTask->blockEvent));
      delete actualTask->blockEvent;
      actualTask->blockEvent = NULL;
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
    cerr << RED("Component::compute( ") <<WHITE(name)<<RED(" ) at time: " << sc_simulation_time()) << endl;
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
  void Component::compute(p_struct* pcb){ 
    p_struct* actualTask = pcb;

#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component::compute( ") <<WHITE(pcb->name)<<RED(" , ")<<WHITE(pcb->funcname)<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *trace_signal=0;
    if(1==trace_map_by_name.count(actualTask->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->name);
      trace_signal=(iter->second);
    }
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
    if( (actualTask->compDelays[this->basename()]).size()>0 && ( (actualTask->compDelays[this->basename()]).count(pcb->funcname) == 0 ) )
  cerr << RED("VPC_LOGICAL_ERROR> ") << YELLOW("having \"functionDelays\" in general, but no delay for this function (")<< pcb->funcname <<YELLOW(")!") << endl;

  std::cerr << "Component> Check if special delay exist for "<< pcb->funcname << " on " << this->basename() << ": " << (actualTask->compDelays[this->basename()]).size() << std::endl;
#endif // VPC_DEBUG
  
    // reset the execution delay
    if( (actualTask->compDelays[this->basename()]).size()>0 && ( (actualTask->compDelays[this->basename()]).count(pcb->funcname) == 1 ) ){
  // function specific delay
  actualTask->remainingDelay = ((actualTask->compDelays[this->basename()])[pcb->funcname]);
#ifdef VPC_DEBUG
  cerr << "Using " << actualTask->remainingDelay << " as delay for function " << pcb->funcname << "!" << endl;
#endif // VPC_DEBUG
    } else {
  // general delay for actor
  actualTask->remainingDelay = actualTask->delay;
#ifdef VPC_DEBUG
  cerr << "Using standard delay " << pcb->remainingDelay << " as delay for function " << pcb->funcname << "!" << endl;
#endif // VPC_DEBUG  
    }
  
    //*********************************************************
    // * SECTION FROM OLD METHOD COMPUTE(P_STRUCT ACTUAL_TASK)
    // *********************************************************/
    
  
  #ifndef NO_VCD_TRACES
    if(1==trace_map_by_name.count(pcb->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(pcb->name);
      pcb->traceSignal=(iter->second);
    }
  #endif //NO_VCD_TRACES


    //int process=pcb->pid;

    // register start of task
    pcb->state=starting;
    Director::getInstance().checkConstraints();
    pcb->state=aktiv;

    //store added task
    newTasks.push_back(pcb);

    //awake scheduler thread
    notify(notify_scheduler_thread);
    //wait(SC_ZERO_TIME);

    ////////////////////////////////////////////////
    //events.push_back(pcb->blockEvent);//
    //notify(notify_scheduler_thread);            //
    //return;                                     //
    ////////////////////////////////////////////////
  }

  void Component::killAllTasks(){
    
    // kill all running tasks
    std::map<int,p_struct* >::iterator iter;
    for(iter = this->runningTasks.begin(); iter != this->runningTasks.end(); iter++){
      
      iter->second->state = activation_state(aborted);
      
#ifdef VPC_DEBUG
        cerr << this->basename() << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->name
        << " activation state set to "<< iter->second->state << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

      scheduler->removedTask(iter->second);
      this->parentControlUnit->signalTaskEvent(iter->second);
      
      // reset pcb  
      iter->second->delay = 0;
      iter->second->remainingDelay = 0;
      
#ifdef VPC_DEBUG
        cerr << this->basename() << " PID: " <<  iter->second->pid << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->name << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

#ifndef NO_VCD_TRACES
        if(iter->second->traceSignal != 0){
          *(iter->second->traceSignal) = S_KILLED;
        }     
#endif //NO_VCD_TRACES
    
    }
    
    //clear all entries
    this->runningTasks.clear();
    
    // kill all ready tasks
    for(iter = this->readyTasks.begin(); iter != this->readyTasks.end(); iter++){
      iter->second->state = activation_state(aborted);

#ifdef VPC_DEBUG
        cerr << this->basename() << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->name
        << " activation state set to "<< iter->second->state << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
      
      scheduler->removedTask(iter->second);
      
      // reset pcb  
      iter->second->delay = 0;
      iter->second->remainingDelay = 0;
      
      this->parentControlUnit->signalTaskEvent(iter->second);

#ifdef VPC_DEBUG
        cerr << this->basename() << " PID: " <<  iter->second->pid << " > ";
        cerr << this->basename() << " killed Task: " << iter->second->name << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG

#ifndef NO_VCD_TRACES
        if(iter->second->traceSignal != 0){
          *(iter->second->traceSignal) = S_KILLED;
        }     
#endif //NO_VCD_TRACES

    }

    this->readyTasks.clear();
 
    // finally check if also new task have to be removed
    while(newTasks.size()>0){
 
      p_struct *newTask;
      newTask = newTasks.front();
      newTasks.pop_front();
      newTask->state = activation_state(aborted);

#ifdef VPC_DEBUG
        cerr << this->basename() << " > ";
        cerr << this->basename() << " killed Task: " << newTask->name
        << " activation state set to "<< newTask->state << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
 
      //reset pcb
      newTask->delay = 0;
      newTask->remainingDelay = 0;
      
      this->parentControlUnit->signalTaskEvent(newTask);
      
#ifndef NO_VCD_TRACES
      *(newTask->traceSignal)=S_KILLED;     
#endif //NO_VCD_TRACES
                
      }
      
  }
  
  void Component::setTraceSignalReadyTasks(trace_value value){
      
      std::map<int, p_struct* >::iterator iter;
      for(iter = this->readyTasks.begin(); iter != this->readyTasks.end(); iter++){

        if(iter->second->traceSignal != 0){
          *(iter->second->traceSignal) = value;
        }     

      }
      
  }
  
  /**************************/
  /*  END OF EXTENSION      */
  /**************************/

} //namespace SystemC_VPC
