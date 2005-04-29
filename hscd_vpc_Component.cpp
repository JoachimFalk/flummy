#include "hscd_vpc_Component.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_RateMonotonicScheduler.h"
#include "hscd_vpc_RoundRobinScheduler.h"
#include "hscd_vpc_PriorityScheduler.h"
#include "hscd_vpc_datatypes.h"
char* AbstractComponent::NAMES[3]={"RISC1","RISC2","RISC3"};

void Component::compute( const char *name ) {
    std::cout << "VPC says: PG node " << name << " start execution " << sc_simulation_time() << std::endl;
    wait( 10, SC_NS);
    std::cout << "VPC says: PG node " << name << " stop execution " << sc_simulation_time() << std::endl;
  }

void Component::compute(int process){
  sc_event interupt;
  p_struct actualTask;
  action_struct *cmd;
  double last_delta_start_time;
  double rest_of_delay;
  bool task_is_running=false;


  actualTask = Director::getInstance().getProcessControlBlock(process);
  actualTask.interupt = &interupt; 
  rest_of_delay=actualTask.delay;
  new_tasks[process]=actualTask;
  cmd = new action_struct;
  cmd->target_pid = process;
  cmd->command = add;
  open_commands.push_back(*cmd);               //          add
  
  

  cout <<"New Task: "<< actualTask.name <<" at: "<< sc_simulation_time() << endl; 
  sc_event& e = scheduler->getNotifyEvent();
  notify(e);
  while(1){
    if(task_is_running){
      last_delta_start_time=sc_simulation_time();
      trace_signal[process]=true;     
      cout << actualTask.name << " is running! "<< sc_simulation_time() << endl; 
      wait(rest_of_delay,SC_NS,*actualTask.interupt);
      cout << actualTask.name << " is stoped! "<< sc_simulation_time() << endl; 
      trace_signal[process]=false;
      rest_of_delay-=sc_simulation_time()-last_delta_start_time;
      if(rest_of_delay==0){ // Process beim Scheduler abmelden
	cmd=new action_struct;
	cmd->target_pid = process;
	cmd->command = retire;
	open_commands.push_back(*cmd);
	notify(e);    //Muss auf den Scheduler gewarted werden? (Nein)
	return;
      }else{}     //Scheduler fragen: Was ist los?
      
    }else{
	wait(*actualTask.interupt);  // Scheduler abwarten
	//Scheduler fragen: Was ist los?
	
    }
    //Scheduler fragen: Was ist los?
    action_struct *cmd;
    cmd=scheduler->getNextNewCommand(process);
    if(NULL != cmd){
      switch(cmd->command){
      case retire:
	//cerr << "retire" << endl;
	// Kann nicht sein
	break;
      case add: 
	//cerr << "add" << endl;
	// Ok aber keine Information, besser: Nichts zu tun!
	break;
      case assign:
	//cerr << "assign" << endl;
	task_is_running=true;
	break;
      case resign:
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
vector<action_struct> &Component::getNewCommands() {
  return open_commands;
}

map<int,p_struct> &Component::getNewTasks() {
  return new_tasks;
}
/*map<int,p_struct> Component::getRunningTasks(){
  return running_tasks;
  }*/


Component::Component(int id){
  //  open_commands=new vector<action_struct>;
  //  new_tasks=new map<int,p_struct>;
  mutex=0;
  this->name= AbstractComponent::NAMES[id];
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

  scheduler->registerComponent(this);
  this->trace = sc_create_vcd_trace_file (this->name); ///////////////////////
  int i;
  for(i=0;i<27;i++){
    sc_trace(this->trace,trace_signal[i],Director::PROCESS[i]);
  }/*
  sc_trace(this->trace,trace_signal[0],"RLC");
sc_trace(this->trace,trace_signal[1],"C_IN_DIFF");
sc_trace(this->trace,trace_signal[2],"C_Q_IQ");
sc_trace(this->trace,trace_signal[3],"SF");
sc_trace(this->trace,trace_signal[4],"C_RF_BM");
sc_trace(this->trace,trace_signal[5],"IN");
sc_trace(this->trace,trace_signal[6],"DCT");
sc_trace(this->trace,trace_signal[7],"C_REC_SF");
sc_trace(this->trace,trace_signal[8],"C_SF_RLC");
sc_trace(this->trace,trace_signal[9],"DIFF");
sc_trace(this->trace,trace_signal[10],"C_IDCT_REC");
sc_trace(this->trace,trace_signal[11],"Q");
sc_trace(this->trace,trace_signal[12],"C_DCT_Q");
sc_trace(this->trace,trace_signal[13],"C_Q_RLC");
sc_trace(this->trace,trace_signal[14],"IDCT");
sc_trace(this->trace,trace_signal[15],"C_DIFF_DCT");
sc_trace(this->trace,trace_signal[16],"C_IN_BM");
sc_trace(this->trace,trace_signal[17],"LF");
sc_trace(this->trace,trace_signal[18],"C_IN_RF");
sc_trace(this->trace,trace_signal[19],"BM");
sc_trace(this->trace,trace_signal[20],"IQ");
sc_trace(this->trace,trace_signal[21],"C_IQ_IDCT");
sc_trace(this->trace,trace_signal[22],"REC");
sc_trace(this->trace,trace_signal[23],"C_LF_REC");
sc_trace(this->trace,trace_signal[24],"C_BM_LF");
sc_trace(this->trace,trace_signal[25],"RF");
sc_trace(this->trace,trace_signal[26],"C_LF_DIFF");

   */
// cout << "<setup component=\"" << this->name << "\"/>" << endl;
}

Component::~Component(){
  // cout << "<kill component=\"" << this->name << "\"/>" << endl;
  sc_close_vcd_trace_file(this->trace);
  //  delete open_commands;
  //  delete new_tasks;
  delete scheduler;
}
