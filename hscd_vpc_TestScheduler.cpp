#include "hscd_vpc_TestScheduler.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"

void TestScheduler::registerComponent(Component *comp){
  this->component=comp;
  open_commands=new map<int,action_struct>;
  //  ready_tasks=new map<int,p_struct>;
  //running_tasks=new map<int,p_struct>;
}

void TestScheduler::schedule_thread(){
  map<int,p_struct> *newTasks;
  action_struct cmd;
  vector<action_struct> *actions;
  sc_time timeslice;
  while(1){
    if(getSchedulerTimeSlice(timeslice)){
      wait(timeslice,notify_scheduler);
    }else{
      wait(notify_scheduler);
    }
    deque<int>::iterator iter;
    newTasks=&component->getNewTasks();   // neue Tasks
    actions=&component->getNewCommands(); // Kommandos
    while(actions->size()){
      cmd=actions->at(actions->size()-1); // letztes kommando
      if(cmd.command==add){               // was ist zu tun
	ready_tasks[cmd.target_pid]=(*newTasks)[cmd.target_pid]; // übername in ready liste
	newTasks->erase(cmd.target_pid);
	addedNewTask(cmd.target_pid);
      }
      else if(cmd.command==retire){    // aus allen listen entfernen!
	if(ready_tasks.find(cmd.target_pid)==ready_tasks.end()){ 
	  if(running_tasks.find(cmd.target_pid)!=running_tasks.end()){ 
	    running_tasks.erase(cmd.target_pid);
	    removedTask(cmd.target_pid);

	  }
	}else{
	  ready_tasks.erase(cmd.target_pid);
	  removedTask(cmd.target_pid);
	
	}
      } //else if(...)
      actions->pop_back();                // Kommando aus Liste entfernen
    } 

    int task_to_resign, task_to_assign;
    scheduling_decision decision=schedulingDecision(task_to_resign, task_to_assign);
    if(decision!=resigned){

      running_tasks[task_to_assign]=ready_tasks[task_to_assign];   //neuen von ready
      ready_tasks.erase(task_to_assign);                              //auf running setzen

      action_struct cmd2;
      cmd2.target_pid=task_to_assign;
      cmd2.command=assign;
      (*open_commands)[task_to_assign]=cmd2;
      
      if(decision==preempt){
	ready_tasks[task_to_resign]=running_tasks[task_to_resign];       //running  -> ready
	running_tasks.erase(task_to_resign);                           //nicht mehr ready
	
	action_struct cmd1;
	cmd1.target_pid=task_to_resign;
	cmd1.command=resign;
	(*open_commands)[task_to_resign]=cmd1;
	
	
	notify(SC_ZERO_TIME,*(ready_tasks[task_to_resign].interupt));
	
      }
      notify(SC_ZERO_TIME,*(running_tasks[task_to_assign].interupt));
      
    }
    
    
    
  }    
    
}
action_struct* TestScheduler::getNextNewCommand(int pid){
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

sc_event& TestScheduler::getNotifyEvent(){
  return notify_scheduler;
}

TestScheduler::~TestScheduler(){
  delete open_commands;
}


