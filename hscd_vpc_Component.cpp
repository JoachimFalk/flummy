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
//#include <hscd_vpc_SchedulerProxy.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_FCFSScheduler.h>
#include <hscd_vpc_RoundRobinScheduler.h>
#include <hscd_vpc_PriorityScheduler.h>
#include <hscd_vpc_RateMonotonicScheduler.h>
#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_Director.h>

//FIXME: Hack TraceLog only exists in SysteMoc <- this is only needed for InfiniBand HCA "hybrid mode"
#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif
//FIXME: end of hack

#include <cosupport/systemc_support.hpp>

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
      sc_time startTime=sc_time_stamp();
      if(!newTaskDuringOverhead){ 
	if(runningTasks.size()<=0){                    // no running task
	  if(hasTimeSlice){                           
	    wait(timeslice - (*overhead), notify_scheduler_thread);
	  }else{
	    wait(notify_scheduler_thread);
	  }
	}else{                                        // a task allready runs
	  if(hasTimeSlice && (timeslice - (*overhead)) < actualRemainingDelay){ 
	    wait(timeslice - (*overhead), notify_scheduler_thread);
	  }else{
	    wait(actualRemainingDelay, notify_scheduler_thread);
	  }
	  sc_time runTime=sc_time_stamp()-startTime;
	  assert(runTime.value()>=0);
	  actualRemainingDelay-=runTime;
	  assert(actualRemainingDelay.value()>=0);
	  if(actualRemainingDelay.value()==0){
	    // all execution time simulated -> BLOCK running task.
	    p_struct *task=runningTasks[actualRunningPID];

	    task->state=ending;
	    Director::getInstance().checkConstraints();
	    task->state=inaktiv;

#ifdef VPC_DEBUG
	    cerr << "PID: " << actualRunningPID<< " > ";
	    cerr << "removed Task: " << task->name << " on: "<< componentName << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
	    notify(*(task->blockEvent));
	    scheduler->removedTask(task);
#ifndef NO_VCD_TRACES
	    if(task->traceSignal!=0) *(task->traceSignal)=S_BLOCKED;     
#endif //NO_VCD_TRACES
	    runningTasks.erase(actualRunningPID);
	    wait(SC_ZERO_TIME);
	  }
	}
      }else{
	  newTaskDuringOverhead=false;
      }

      //look for new tasks (they called compute)
      while(newTasks.size()>0){
	p_struct *newTask;
	newTask=newTasks[0];
	newTasks.pop_front();
#ifdef VPC_DEBUG
	cerr<< "received new Task: " << newTask->name << " on: "<< componentName << " at: " << sc_simulation_time() << endl;
#endif // VPCDEBUG
#ifndef NO_VCD_TRACES
	*(newTask->traceSignal)=S_READY;     
#endif //NO_VCD_TRACES
	//insert new task in read list
	assert(readyTasks.find(newTask->pid)   == readyTasks.end()   /* An task can call compute only one time! */);
	assert(runningTasks.find(newTask->pid) == runningTasks.end() /* An task can call compute only one time! */);
	
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
      //if( overhead != NULL ) delete overhead;
      overhead=scheduler->schedulingOverhead();

      if( overhead != NULL ){
	schedulerTrace = S_RUNNING;
	//    actual time    < endtime
	while( sc_time_stamp() < timestamp + (*overhead) ){
	  wait((timestamp+(*overhead))-sc_time_stamp(), notify_scheduler_thread);
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
	actualRunningPID=taskToAssign;
#ifdef VPC_DEBUG
	cerr << "PID: " << taskToAssign;
	cerr << "> remaining delay for " << runningTasks[taskToAssign]->name;
#endif // VPCDEBUG
	actualRemainingDelay=sc_time(runningTasks[taskToAssign]->remainingDelay,SC_NS);
#ifdef VPC_DEBUG
	cerr<< " is " << runningTasks[taskToAssign]->remainingDelay << " on: "<< componentName << " at: " << sc_simulation_time() << endl;
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
  void Component::compute(p_struct *actualTask){


#ifndef NO_VCD_TRACES
    if(1==trace_map_by_name.count(actualTask->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->name);
      actualTask->traceSignal=(iter->second);
    }
#endif //NO_VCD_TRACES


    //int process=actualTask->pid;

    // register start of task
    actualTask->state=starting;
    Director::getInstance().checkConstraints();
    actualTask->state=aktiv;

  //store added task
    newTasks.push_back(actualTask);

    //awake scheduler thread
    notify(notify_scheduler_thread);
    //    wait(SC_ZERO_TIME);

    ////////////////////////////////////////////////
    //events.push_back(actualTask->blockEvent);//
    //notify(notify_scheduler_thread);            //
    //return;                                     //
    ////////////////////////////////////////////////
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
  void Component::compute( const char *name, const char *funcname, CoSupport::SystemC::Event *end) { 
    p_struct  *actualTask = Director::getInstance().getProcessControlBlock(name);
    actualTask->blockEvent=end;

#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component::compute( ") <<WHITE(name)<<RED(" , ")<<WHITE(funcname)<<RED(" ) at time: " << sc_simulation_time()) << " on: "<< componentName << endl;
#endif

#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *trace_signal=0;
    if(1==trace_map_by_name.count(actualTask->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->name);
      trace_signal=(iter->second);
    }
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
    if( actualTask->functionDelays.size()>0 && ( actualTask->functionDelays.count(funcname) == 0 ) )
	cerr << RED("VPC_LOGICAL_ERROR> ") << YELLOW("having \"functionDelays\" in general, but no delay for this function (")<< funcname <<YELLOW(")!") << endl;
#endif // VPC_DEBUG
  
    // reset the execution delay
    if( actualTask->functionDelays.size()>0 && ( actualTask->functionDelays.count(funcname) == 1 ) ){
	// function specific delay
	actualTask->remainingDelay = (actualTask->functionDelays[funcname]);
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
        //FIXME: (Hack) TraceLog only exists in SysteMoc <- this is only needed for InfiniBand HCA "hybrid mode"
        //assume this is an smoc1 actor
#ifdef SYSTEMOC_TRACE
        TraceLog << "<compute actor=\""<<name<<"\" function=\""<<funcname<<"\"/>" << std::endl;
#endif
	// active mode -> returns if simulated delay time has expired (blocking compute call)
	actualTask->blockEvent = new CoSupport::SystemC::Event();
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
  void Component::compute( const char *name, CoSupport::SystemC::Event *end) { 
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
} //namespace SystemC_VPC
