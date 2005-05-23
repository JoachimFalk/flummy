#include "hscd_vpc_RateMonotonicScheduler.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"
#include <float.h>

void RateMonotonicScheduler::registerComponent(Component *comp){
  this->component=comp;
  open_commands=new map<int,action_struct>;
  //  ready_tasks=new map<int,p_struct>;
  //running_tasks=new map<int,p_struct>;
}

void RateMonotonicScheduler::schedule_thread(){
  map<int,p_struct> *newTasks;
  action_struct cmd;
  vector<action_struct> *actions;
  while(1){
    wait(notify_scheduler);
    newTasks=&component->getNewTasks();   // neue Tasks
    actions=&component->getNewCommands(); // Kommandos

    while(actions->size()){
      cmd=actions->at(actions->size()-1); // letztes kommando
      if(cmd.command==ADD){               // was ist zu tun
	ready_tasks[cmd.target_pid]=(*newTasks)[cmd.target_pid]; // übername in ready liste
	newTasks->erase(cmd.target_pid);
      }
      else if(cmd.command==RETIRE){    // aus allen listen entfernen!
	if(ready_tasks.find(cmd.target_pid)==ready_tasks.end()){ 
	  if(running_tasks.find(cmd.target_pid)!=running_tasks.end()){ 
	    running_tasks.erase(cmd.target_pid);

	  }
	}else{
	  ready_tasks.erase(cmd.target_pid);
	}
	
      } //else if(...)
      actions->pop_back();                // Kommando aus Liste entfernen
    } 
    //Task mit größter Priorität (kleinste rate) bestimmen:
    double min_rate=DBL_MAX;
    int min_rate_pid;
    map<int,p_struct>::iterator iter;
    p_struct *pcb;
    for (iter=ready_tasks.begin(); iter!=ready_tasks.end(); ++iter){
      pcb=&iter->second; ///////////////////////////77
      
      if(min_rate>(pcb->delay / pcb->period)){
	min_rate=(pcb->delay / pcb->period);
	min_rate_pid=pcb->pid;
      }
    }

    if(running_tasks.size()>0){
      /*  double min_rate=DOUBLE_MIN;
      int min_rate_pid;
      for (iter=running_tasks.begin(); iter!=running_tasks.end(); ++iter){
	p_struct pcb=iter->second;
	if(min_rate>pcb.rate){
	  min_rate=pcb.rate;
	  min_rate_pid=pcb.pid;
	}
	}*/
      iter=running_tasks.begin();
      p_struct pcb=iter->second;
      if((pcb.delay / pcb.period)>min_rate){//neuer Task mit höherer Priorität!
	running_tasks.erase(pcb.pid);       //Alten Task von running auf
 	ready_tasks[pcb.pid]=pcb;            //ready setzen
	action_struct cmd1;
	cmd1.target_pid=pcb.pid;
	cmd1.command=RESIGN;
	(*open_commands)[pcb.pid]=cmd1;
	
	running_tasks[min_rate_pid]=ready_tasks[min_rate_pid];   //neuen von ready
	ready_tasks.erase(min_rate_pid);                              //auf running setzen
	action_struct cmd2;
	cmd2.target_pid=min_rate_pid;
	cmd2.command=ASSIGN;
	(*open_commands)[min_rate_pid]=cmd2;

	notify(SC_ZERO_TIME,*pcb.interupt);
	notify(SC_ZERO_TIME,*(running_tasks[min_rate_pid].interupt));

      } 
    }else{  //kein Task auf running!
      if(min_rate<DBL_MAX){
      running_tasks[min_rate_pid]=ready_tasks[min_rate_pid];   //neuen von ready
      action_struct cmd2;
      cmd2.target_pid=min_rate_pid;
      cmd2.command=ASSIGN;
      (*open_commands)[min_rate_pid]=cmd2;
      notify(SC_ZERO_TIME,*(running_tasks[min_rate_pid].interupt));
      ready_tasks.erase(min_rate_pid);                             //auf running setzen
      }
    }
    

  }
}
action_struct* RateMonotonicScheduler::getNextNewCommand(int pid){
  map<int,action_struct>::iterator it;
  it=open_commands->find(pid);
  if(it==open_commands->end()){   // kein Komando
    return NULL;
  }else{
    action_struct *action = &it->second;
    open_commands->erase(it);
    return action;
  }
}

sc_event& RateMonotonicScheduler::getNotifyEvent(){
  return notify_scheduler;
}

RateMonotonicScheduler::~RateMonotonicScheduler(){
  delete open_commands;
}
