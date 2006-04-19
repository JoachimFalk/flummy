#include <hscd_vpc_PriorityScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>
#include <hscd_vpc_datatypes.h>

namespace SystemC_VPC{
  PriorityScheduler::PriorityScheduler(const char *schedulername){

    priority_queue<p_queue_entry,vector<p_queue_entry>,p_queue_compare> pqueue(comp);

    order_counter=0;

    char rest[VPC_MAX_STRING_LENGTH];
    int sublength;
    char *secondindex;
    char *firstindex=strchr(schedulername,':');    //':' finden -> ':' trennt key-value Paare 
    while(firstindex!=NULL){
      secondindex=strchr(firstindex+1,':');        //':' überspringen und nächste ':' finden
      if(secondindex!=NULL)
  sublength=secondindex-firstindex;          //Länge bestimmen
      else
  sublength=strlen(firstindex);              
      strncpy(rest,firstindex+1,sublength-1);      //key-value extrahieren
      rest[sublength-1]='\0';
      firstindex=secondindex;                     
    
    
      char *key, *value;               // key und value trennen und Property setzen
      value=strstr(rest,"-");
      if(value!=NULL){
  value[0]='\0';
  value++;
  key=rest;
  setProperty(key,value);
      }
    
    }
  }

  void PriorityScheduler::setProperty(char* key, char* value){
  }

  bool PriorityScheduler::getSchedulerTimeSlice(sc_time& time,const map<int,ProcessControlBlock*> &ready_tasks,const  map<int,ProcessControlBlock*> &running_tasks){
     return false;
  }
  /**
   *
   */  void PriorityScheduler::addedNewTask(ProcessControlBlock *pcb){
    p_queue_entry pqe;
    pqe.fifo_order=order_counter++;
    pqe.pcb=pcb;
    pqueue.push(pqe);
  }
  /**
   *
   */  void PriorityScheduler::removedTask(ProcessControlBlock *pcb){
  }

  /**
   *
   */
   scheduling_decision PriorityScheduler::schedulingDecision(int& task_to_resign, int& task_to_assign,const  map<int,ProcessControlBlock*> &ready_tasks,const  map<int,ProcessControlBlock*> &running_tasks){
     scheduling_decision ret_decision=ONLY_ASSIGN;
     if(pqueue.size()<=0) return NOCHANGE;    // kein neuer -> nichts tun
     p_queue_entry prior_ready=pqueue.top();  // höchste priorität der ready tasks
     double d_prior_ready=prior_ready.pcb->getPriority();  // wert der priorität
     task_to_assign=prior_ready.pcb->getPID();


     if(running_tasks.size()!=0){  // läuft noch einer ?
       map<int,ProcessControlBlock*>::const_iterator iter;
       iter=running_tasks.begin();
       ProcessControlBlock *pcb=iter->second;
       if(pcb->getPriority() <= d_prior_ready){             //laufender mit höherer oder gleicher priorität ->
         ret_decision=NOCHANGE;                       //nicht verdrängen
       }else{
         ret_decision=PREEMPT;                        //verdrängen
         task_to_resign=pcb->getPID(); 
         pqueue.pop();
         p_queue_entry pqe={0,pcb};
         pqueue.push(pqe);
       }
     }else{
       pqueue.pop();
       ret_decision=ONLY_ASSIGN;  
     }


     return ret_decision;
   }
}
