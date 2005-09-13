#include <hscd_vpc_SchedulerProxy.h>
#include <hscd_vpc_FCFSScheduler.h>
#include <hscd_vpc_RoundRobinScheduler.h>
#include <hscd_vpc_PriorityScheduler.h>
#include <hscd_vpc_RateMonotonicScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>
namespace SystemC_VPC{
  void SchedulerProxy::registerComponent(Component *comp){
    this->component=comp;
    open_commands=new map<int,action_struct>;
    //  ready_tasks=new map<int,p_struct>;
    //running_tasks=new map<int,p_struct>;
  }
  void  SchedulerProxy::setScheduler(const char *schedulername){
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


  void SchedulerProxy::schedule_thread(){
    map<int,p_struct*> *newTasks;
    action_struct cmd;
    vector<action_struct> *actions;
    sc_time timeslice;
    while(1){
      if(scheduler->getSchedulerTimeSlice(timeslice,ready_tasks,running_tasks)){
	wait(timeslice,notify_scheduler);
      }else{
	wait(notify_scheduler);
      }
      //     deque<int>::iterator iter;
      newTasks=&component->getNewTasks();   // neue Tasks
      actions=&component->getNewCommands(); // Kommandos
      while(actions->size()){
	cmd=actions->at(actions->size()-1); // letztes kommando
	if(cmd.command==READY){               // was ist zu tun
	  //	cerr << "ready" <<endl;
	  ready_tasks[cmd.target_pid]=(*newTasks)[cmd.target_pid]; // übername in ready liste
	  newTasks->erase(cmd.target_pid);
	  scheduler->addedNewTask(ready_tasks[cmd.target_pid]);
	}
	else if(cmd.command==BLOCK){    // aus allen listen entfernen!
	  //  cerr << "remove" <<endl;

	  if(ready_tasks.find(cmd.target_pid)==ready_tasks.end()){ 
	    if(running_tasks.find(cmd.target_pid)!=running_tasks.end()){ 
	      p_struct *pcb_to_remove=running_tasks[cmd.target_pid];
	      running_tasks.erase(cmd.target_pid);
	      scheduler->removedTask(pcb_to_remove);
	
	    }
	  }else{
	    p_struct *pcb_to_remove=ready_tasks[cmd.target_pid];
	    ready_tasks.erase(cmd.target_pid);
	    scheduler->removedTask(pcb_to_remove);
	
	  }
	} //else if(...)
	actions->pop_back();                // Kommando aus Liste entfernen
      } 

      int task_to_resign, task_to_assign;
      scheduling_decision decision=scheduler->schedulingDecision(task_to_resign, task_to_assign,ready_tasks,running_tasks);
      if(decision != NOCHANGE){ //nichts tun
	if(decision!=RESIGNED){ // zZ auch nichts  tun! keine Thread mehr da, und Listen sind schon gereinigt!
	  //FIXME: error if using TDMA Scheduling!!!

	  running_tasks[task_to_assign]=ready_tasks[task_to_assign];   //neuen von ready
	  ready_tasks.erase(task_to_assign);                              //auf running setzen
	
	  action_struct cmd2;
	  cmd2.target_pid=task_to_assign;
	  cmd2.command=ASSIGN;
	  (*open_commands)[task_to_assign]=cmd2;
	
	  if(decision==PREEMPT){
	    ready_tasks[task_to_resign]=running_tasks[task_to_resign];       //running  -> ready
	    running_tasks.erase(task_to_resign);                           //nicht mehr ready
	  
	    action_struct cmd1;
	    cmd1.target_pid=task_to_resign;
	    cmd1.command=RESIGN;
	    (*open_commands)[task_to_resign]=cmd1;
	  
	  
	    notify(SC_ZERO_TIME,*(ready_tasks[task_to_resign]->interupt));
	  }
	  notify(SC_ZERO_TIME,*(running_tasks[task_to_assign]->interupt));
	}
      }
    }    
  }
  action_struct* SchedulerProxy::getNextNewCommand(int pid){
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

  sc_event& SchedulerProxy::getNotifyEvent(){
    return notify_scheduler;
  }

  SchedulerProxy::~SchedulerProxy(){
    delete open_commands;
    delete scheduler;
  }
  /**
   *
   */
  void SchedulerProxy::processAndForwardParameter(char *sType,char *sValue){
    scheduler->setProperty(sType,sValue);
  }
}

