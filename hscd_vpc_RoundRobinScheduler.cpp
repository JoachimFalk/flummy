#include "hscd_vpc_RoundRobinScheduler.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"

void RoundRobinScheduler::registerComponent(Component *comp){
  this->component=comp;
  open_commands=new map<int,action_struct>;
  //  ready_tasks=new map<int,p_struct>;
  //running_tasks=new map<int,p_struct>;
}
void RoundRobinScheduler::schedule(int process){}
void RoundRobinScheduler::schedule_thread(){
  map<int,p_struct> *newTasks;
  action_struct cmd;
  vector<action_struct> *actions;
  while(1){
    wait(TIMESLICE,SC_NS,notify_scheduler);
    deque<int>::iterator iter;
	for(iter=rr_fifo.begin();iter!=rr_fifo.end();iter++){
	}
    newTasks=&component->getNewTasks();   // neue Tasks
    actions=&component->getNewCommands(); // Kommandos
    //    deque<int>::iterator iter;
    
    while(actions->size()){
      cmd=actions->at(actions->size()-1); // letztes kommando
      if(cmd.command==add){               // was ist zu tun
	ready_tasks[cmd.target_pid]=(*newTasks)[cmd.target_pid]; // übername in ready liste
	newTasks->erase(cmd.target_pid);

	rr_fifo.push_back(cmd.target_pid);
 
     }
      else if(cmd.command==retire){    // aus allen listen entfernen!
	if(ready_tasks.find(cmd.target_pid)==ready_tasks.end()){ 
	  if(running_tasks.find(cmd.target_pid)!=running_tasks.end()){ 
	    running_tasks.erase(cmd.target_pid);
	    
	  }
	}else{
	  ready_tasks.erase(cmd.target_pid);
	}
	for(iter=rr_fifo.begin();iter!=rr_fifo.end();iter++){
	  if( *iter == cmd.target_pid){

	    rr_fifo.erase(iter);

	    break;
	  }
	}
      } //else if(...)
      actions->pop_back();                // Kommando aus Liste entfernen
    } 
    if(rr_fifo.size()>0){

      int rr_new_task = rr_fifo.front();
      rr_fifo.pop_front();


      if(running_tasks.size()>0){
	map<int,p_struct>::iterator iter2;
	iter2=running_tasks.begin();
	p_struct pcb=iter2->second;
	running_tasks.erase(pcb.pid);       //Alten Task von running auf
	ready_tasks[pcb.pid]=pcb;            //ready setzen

	rr_fifo.push_back(pcb.pid);
  
	action_struct cmd1;
	cmd1.target_pid=pcb.pid;
	cmd1.command=resign;
	(*open_commands)[pcb.pid]=cmd1;
	running_tasks[rr_new_task]=ready_tasks[rr_new_task];   //neuen von ready
	ready_tasks.erase(rr_new_task);                              //auf running setzen
	action_struct cmd2;
	cmd2.target_pid=rr_new_task;
	cmd2.command=assign;
	(*open_commands)[rr_new_task]=cmd2;
	
	notify(*pcb.interupt);
	notify(*(running_tasks[rr_new_task].interupt));
	
      }else{  //kein Task auf running!
	running_tasks[rr_new_task]=ready_tasks[rr_new_task];   //neuen von ready
	action_struct cmd2;
	cmd2.target_pid=ready_tasks[rr_new_task].pid;
	cmd2.command=assign;
	(*open_commands)[rr_new_task]=cmd2;
	notify(*(running_tasks[rr_new_task].interupt));
	ready_tasks.erase(rr_new_task);                             //auf running setzen
      }
    }
  }    
    
}
action_struct* RoundRobinScheduler::getNextNewCommand(int pid){
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

sc_event& RoundRobinScheduler::getNotifyEvent(){
  return notify_scheduler;
}

RoundRobinScheduler::~RoundRobinScheduler(){
  delete open_commands;
}
