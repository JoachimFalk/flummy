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
#include <hscd_vpc_SchedulerProxy.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_Director.h>

#include <smoc_event.hpp>

#include <values.h>
namespace SystemC_VPC{

  /**
   *
   */
  void Component::compute( const char *name, const char *funcname, smoc_event *end) { 
    p_struct  *actualTask = Director::getInstance().getProcessControlBlock(name);
    actualTask->smoc_interupt=end;

#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component::compute(") <<WHITE(name)<<RED(" , ")<<WHITE(funcname)<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *trace_signal=0;
    if(1==trace_map_by_name.count(actualTask->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->name);
      trace_signal=(iter->second);
    }
    if (trace_signal != NULL ) {
      *trace_signal = S_READY;
    }
#endif //NO_VCD_TRACES

    compute(actualTask);

#ifndef NO_VCD_TRACES
    if (trace_signal != NULL ) {
      *trace_signal = S_BLOCKED;
    }
#endif //NO_VCD_TRACES
  }

  /**
   *
   */
  void Component::compute( const char *name, smoc_event *end) { 
#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("Component::compute(") <<WHITE(name)<<RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

    compute(name,"",end);
  }

  /**
   *
   */
  void Component::compute(p_struct *actualTask){
    sc_event interupt;
    action_struct *cmd;
    double last_delta_start_time;
    double rest_of_delay;
    bool task_is_running=false;
    int process=actualTask->pid;

    actualTask->state=starting;
    Director::getInstance().checkConstraints();
    actualTask->state=aktiv;

#ifndef NO_VCD_TRACES
    sc_signal<trace_value> *trace_signal=NULL;
    if(1==trace_map_by_name.count(actualTask->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->name);
      trace_signal=(iter->second);
    }
#endif //NO_VCD_TRACES


    actualTask->interupt = &interupt;
    // reset the execution delay
    rest_of_delay=actualTask->delay;
    newTasks[process]=actualTask;
    cmd = new action_struct;
    cmd->target_pid = process;
    cmd->command = READY;
    open_commands.push_back(*cmd);               //          add

    sc_event& e = schedulerproxy->getNotifyEvent();
    notify(SC_ZERO_TIME,e);
    while(1){
      if(task_is_running){
	last_delta_start_time=sc_simulation_time();

#ifndef NO_VCD_TRACES
	if(trace_signal!=0)*trace_signal=S_RUNNING;     
#endif //NO_VCD_TRACES

	//   cout << actualTask->name << " is running! "<< sc_simulation_time() << endl; 
	wait(rest_of_delay,SC_NS,*actualTask->interupt);
	//cout << actualTask->name << " is stoped! "<< sc_simulation_time() << endl; 

#ifndef NO_VCD_TRACES
	if(trace_signal!=0)*trace_signal=S_READY;
#endif //NO_VCD_TRACES

	rest_of_delay-=sc_simulation_time()-last_delta_start_time;
	if(rest_of_delay==0){ // Process beim Scheduler abmelden
	  cmd=new action_struct;
	  cmd->target_pid = process;
	  cmd->command = BLOCK;
	  open_commands.push_back(*cmd);
	  notify(e);    //Muss auf den Scheduler gewarted werden? (Nein)
	  wait(SC_ZERO_TIME);
	  break;
	}else{}     //Scheduler fragen: Was ist los?

      }else{
	wait(*actualTask->interupt);  // Scheduler abwarten
	//Scheduler fragen: Was ist los?

      }
      //Scheduler fragen: Was ist los?
      action_struct *cmd;
      cmd=schedulerproxy->getNextNewCommand(process);
      if(NULL != cmd){
	switch(cmd->command){
	case BLOCK:
	  //cerr << "block" << endl;
	  // Kann nicht sein
	  break;
	case READY: 
	  //cerr << "ready" << endl;
	  // Ok aber keine Information, besser: Nichts zu tun!
	  break;
	case ASSIGN:
	  //	cerr << "assign" << endl;
	  task_is_running=true;
	  break;
	case RESIGN:
	  //cerr << "resign" << endl;
	  task_is_running=false;
	  break;
	default:
	  //	cerr << "def" << endl;
	  break;
	}
      }
    }

    actualTask->state=ending;
    Director::getInstance().checkConstraints();
    actualTask->state=inaktiv;


    //newTasks.erase(process);
  }

  /**
   *
   */
  Component::Component(const char *name,const char *schedulername){
    strcpy(this->componentName,name);
    schedulerproxy=new SchedulerProxy(this->componentName);
    schedulerproxy->setScheduler(schedulername);
    schedulerproxy->registerComponent(this);

#ifndef NO_VCD_TRACES
    string tracefilename=this->componentName;
    char tracefilechar[VPC_MAX_STRING_LENGTH];
    char* traceprefix= getenv("VPCTRACEFILEPREFIX");
    if(0!=traceprefix){
      tracefilename.insert(0,traceprefix);
    }
    strcpy(tracefilechar,tracefilename.c_str());
    this->traceFile =sc_create_vcd_trace_file (tracefilechar);
    ((vcd_trace_file*)this->traceFile)->sc_set_vcd_time_unit(-9);
#endif //NO_VCD_TRACES

  }


  /**
   *
   */
  Component::~Component(){

#ifndef NO_VCD_TRACES
    sc_close_vcd_trace_file(this->traceFile);
#endif //NO_VCD_TRACES
    delete schedulerproxy;

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


  /**
   *
   */
  vector<action_struct> &Component::getNewCommands() {
    return open_commands;
  }


  /**
   *
   */
  map<int,p_struct*> &Component::getNewTasks() {
    return newTasks;
  }


  /**
   *
   */
  void ThreadedComponent::schedule_thread(){
    /////////////////////////////////////
    // while(1){
    //  wait(notify_scheduler_thread);
    //   do{
    //     wait(2,SC_NS);
    //     smoc_notify(*(events.front()));
    //     events.pop_front();
    //   }while(events.size()>0);
    // }
    /////////////////////////////////////
    sc_time timeslice;
    sc_time actualRemainingDelay;
    int actualRunningPID;
    while(1){
      //determine the time slice for next scheduling descission and wait for
      bool hasTimeSlice= scheduler->getSchedulerTimeSlice(timeslice, readyTasks,runningTasks);
      sc_time startTime=sc_time_stamp();
      if(runningTasks.size()<=0){                    // no running task
	if(hasTimeSlice){                           
	  wait(timeslice, notify_scheduler_thread);
	}else{
	  wait(notify_scheduler_thread);
	}
      }else{                                        // a task allready runs
	if(hasTimeSlice && timeslice < actualRemainingDelay){ 
	  wait(timeslice, notify_scheduler_thread);
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
	  smoc_notify(*(task->smoc_interupt));
	  scheduler->removedTask(task);
#ifndef NO_VCD_TRACES
	  if(task->traceSignal!=0) *(task->traceSignal)=S_READY;     
#endif //NO_VCD_TRACES
	  runningTasks.erase(actualRunningPID);
	}
      }

      //look for new tasks (they called compute)
      while(newTasks.size()>0){
	p_struct *newTask;
	newTask=newTasks[0];
	newTasks.pop_front();
#ifdef VPC_DEBUG
	cerr<< "received new Task: " << newTask->name << endl;
#endif // VPCDEBUG
	//insert new task in read list
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
	readyTasks[taskToResign]->remaining_delay=actualRemainingDelay.to_default_time_units();
#ifndef NO_VCD_TRACES
	if(readyTasks[taskToResign]->traceSignal!=0) *(readyTasks[taskToResign]->traceSignal)=S_READY;     
#endif //NO_VCD_TRACES
      }

      //assign task
      if(decision==ONLY_ASSIGN || decision==PREEMPT){
	runningTasks[taskToAssign]=readyTasks[taskToAssign];
	readyTasks.erase(taskToAssign);
	actualRunningPID=taskToAssign;
	actualRemainingDelay=sc_time(runningTasks[taskToAssign]->remaining_delay,SC_NS);
#ifndef NO_VCD_TRACES
	if(runningTasks[taskToAssign]->traceSignal!=0) *(runningTasks[taskToAssign]->traceSignal)=S_RUNNING;     
#endif //NO_VCD_TRACES
      }

   
    }
    
  }

 

  /**
   *
   */
  void ThreadedComponent::compute(p_struct *actualTask){

#ifndef NO_VCD_TRACES
    if(1==trace_map_by_name.count(actualTask->name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask->name);
      actualTask->traceSignal=(iter->second);
    }
#endif //NO_VCD_TRACES

#ifdef VPC_DEBUG
    cout << flush;
    cerr << RED("This is a ThreadedComponent!")<< endl;
#endif
    int process=actualTask->pid;

    // register start of task
    actualTask->state=starting;
    Director::getInstance().checkConstraints();
    actualTask->state=aktiv;

    // reset the execution delay
    actualTask->remaining_delay = actualTask->delay;
    
    //store added task
    newTasks.push_back(actualTask);

    //awake scheduler thread
    notify(notify_scheduler_thread);


    ////////////////////////////////////////////////
    //events.push_back(actualTask->smoc_interupt);//
    //notify(notify_scheduler_thread);            //
    //return;                                     //
    ////////////////////////////////////////////////
  }

} //namespace SystemC_VPC
