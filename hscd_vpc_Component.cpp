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
#include "hscd_vpc_Component.h"
#include "hscd_vpc_SchedulerProxy.h"
#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_Director.h"

//char* AbstractComponent::NAMES[3]={"RISC1","RISC2","RISC3"};

void Component::compute( const char *name ) { 
  p_struct  actualTask = Director::getInstance().getProcessControlBlock(name);
  sc_signal<trace_value> *trace_signal=0;
  if(1==trace_map_by_name.count(actualTask.name)){
    map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask.name);
    trace_signal=(iter->second);
  }
  if (trace_signal != NULL ) {
    *trace_signal = READY;
    //std::cerr << "VPC says: PG node " << name << " start execution " << sc_simulation_time() << " on: " << this->name << std::endl;
  }
  //wait((80.0*rand()/(RAND_MAX+1.0)), SC_NS);
  //wait(10, SC_NS);
  compute(actualTask);
  if (trace_signal != NULL ) {
    *trace_signal = BLOCKED;
    // std::cerr << "VPC says: PG node " << name << " stop execution " << sc_simulation_time() << std::endl;
  }
}

/*void Component::compute(int process){
  /////////
  p_struct actualTask = Director::getInstance().getProcessControlBlock(process);
  compute(actualTask);
  }*/
void Component::compute(p_struct actualTask){
  sc_event interupt;
  action_struct *cmd;
  double last_delta_start_time;
  double rest_of_delay;
  bool task_is_running=false;
  int process=actualTask.pid;
  sc_signal<trace_value> *trace_signal=NULL;
  if(1==trace_map_by_name.count(actualTask.name)){
      map<string,sc_signal<trace_value>*>::iterator iter = trace_map_by_name.find(actualTask.name);
      trace_signal=(iter->second);
  }


  actualTask.interupt = &interupt; 
  rest_of_delay=actualTask.delay;
  new_tasks[process]=actualTask;
  cmd = new action_struct;
  cmd->target_pid = process;
  cmd->command = ADD;
  open_commands.push_back(*cmd);               //          add

  sc_event& e = schedulerproxy->getNotifyEvent();
  notify(SC_ZERO_TIME,e);
  while(1){
    if(task_is_running){
      last_delta_start_time=sc_simulation_time();
      if(trace_signal!=0)*trace_signal=RUNNING;     
      //   cout << actualTask.name << " is running! "<< sc_simulation_time() << endl; 
      wait(rest_of_delay,SC_NS,*actualTask.interupt);
      //cout << actualTask.name << " is stoped! "<< sc_simulation_time() << endl; 
      if(trace_signal!=0)*trace_signal=READY;
      rest_of_delay-=sc_simulation_time()-last_delta_start_time;
      if(rest_of_delay==0){ // Process beim Scheduler abmelden
	cmd=new action_struct;
	cmd->target_pid = process;
	cmd->command = RETIRE;
	open_commands.push_back(*cmd);
	notify(e);    //Muss auf den Scheduler gewarted werden? (Nein)
	wait(SC_ZERO_TIME);
	return;
      }else{}     //Scheduler fragen: Was ist los?
      
    }else{
	wait(*actualTask.interupt);  // Scheduler abwarten
	//Scheduler fragen: Was ist los?
	
    }
    //Scheduler fragen: Was ist los?
    action_struct *cmd;
    cmd=schedulerproxy->getNextNewCommand(process);
    if(NULL != cmd){
      switch(cmd->command){
      case RETIRE:
	//cerr << "retire" << endl;
	// Kann nicht sein
	break;
      case ADD: 
	//cerr << "add" << endl;
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
  //new_tasks.erase(process);
} 
Component::Component(const char *name,const char *schedulername){
    strcpy(this->name,name);
    schedulerproxy=new SchedulerProxy(this->name);
    schedulerproxy->setScheduler(schedulername);
    schedulerproxy->registerComponent(this);
    string tracefilename=this->name;
    char tracefilechar[VPC_MAX_STRING_LENGTH];
    char* traceprefix= getenv("VPCTRACEFILEPREFIX");
    if(0!=traceprefix){
	tracefilename.insert(0,traceprefix);
    }
    strcpy(tracefilechar,tracefilename.c_str());
    //cerr << "Trace: "<<tracefilechar <<endl;
    this->trace =sc_create_vcd_trace_file (tracefilechar);//this->name); ///////////////////////
    ((vcd_trace_file*)this->trace)->sc_set_vcd_time_unit(-9);
    //    this->trace_wif = sc_create_wif_trace_file (tracefilechar);//this->name); ///////////////////////
    
    int i;
    for(i=0;i<23;i++){
//    sc_trace(this->trace,trace_signal[i],Director::PROCESS[i]);
    }
}

Component::~Component(){
    cerr << "~Component() " << endl;

  // cout << "<kill component=\"" << this->name << "\"/>" << endl;
  sc_close_vcd_trace_file(this->trace);
  //  delete open_commands;
  //  delete new_tasks;
  delete schedulerproxy;
}
void Component::informAboutMapping(string module){
    sc_signal<trace_value> *newsignal=new sc_signal<trace_value>();
    trace_map_by_name.insert(pair<string,sc_signal<trace_value>*>(module,newsignal));
    sc_trace(this->trace,*newsignal,module.c_str());
    //    sc_trace(this->trace_wif,*newsignal,module.c_str());
}

vector<action_struct> &Component::getNewCommands() {
  return open_commands;
}

map<int,p_struct> &Component::getNewTasks() {
  return new_tasks;
}

/*  old stuff
Component::Component(int id){
    cerr << "Component(int) " << endl;

  //  open_commands=new vector<action_struct>;
  //  new_tasks=new map<int,p_struct>;
  mutex=0;
//  this->name= AbstractComponent::NAMES[id];
  this->id=id;
  // cerr << "Scheduler: " << id << endl;
  if(id==0){
    scheduler=new PriorityScheduler(this->name);
  }else{
    if(id==1){
      scheduler=new RateMonotonicScheduler(this->name);
    }else{
      scheduler=new RoundRobinScheduler(this->name);
    }
  }

  schedulerproxy->registerComponent(this);
  this->trace = sc_create_vcd_trace_file (this->name); ///////////////////////
  int i;
  for(i=0;i<27;i++){
//    sc_trace(this->trace,trace_signal[i],Director::PROCESS[i]);
  }
}
Component::Component(const Component &comp){
    cerr << "Component(const Component) " << endl;
    scheduler=new PriorityScheduler(this->name);

}
Component::Component(){
    cerr << "Component() " << endl;
    scheduler=new PriorityScheduler(this->name);

}

Component::Component(const char *name,int id){
    cerr << "Component(const char,int) " << endl;
    strcpy(this->name,name);
    this->id=id;
    if(id==0){
	scheduler=new PriorityScheduler(this->name);
    }else{
	if(id==1){
	    scheduler=new RateMonotonicScheduler(this->name);
	}else{
	    scheduler=new RoundRobinScheduler(this->name);
	}
    }
    
    schedulerproxy->registerComponent(this);
    this->trace = sc_create_vcd_trace_file (this->name); ///////////////////////
    
    int i;
    for(i=0;i<23;i++){
//    sc_trace(this->trace,trace_signal[i],Director::PROCESS[i]);
    }
}


*/
