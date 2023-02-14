#include <hscd_vpc_RateMonotonicScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>
#include <hscd_vpc_datatypes.h>

namespace SystemC_VPC{
  RateMonotonicScheduler::RateMonotonicScheduler(const char *schedulername){

    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,rm_queue_compare>
      pqueue(comp);

    order_counter=0;

    char rest[VPC_MAX_STRING_LENGTH];
    int sublength;
    char *secondindex;
    //':' finden -> ':' trennt key-value Paare 
    char *firstindex=strchr(schedulername,':');
    while(firstindex!=NULL){

      //':' �berspringen und n�chste ':' finden
      secondindex=strchr(firstindex+1,':');
      if(secondindex!=NULL)
  sublength=secondindex-firstindex;          //L�nge bestimmen
      else
  sublength=strlen(firstindex);              
      strncpy(rest,firstindex+1,sublength-1);      //key-value extrahieren
      rest[sublength-1]='\0';
      firstindex=secondindex;                     
    
      // key und value trennen und Property setzen
      char *key, *value;
      value=strstr(rest,"-");
      if(value!=NULL){
  value[0]='\0';
  value++;
  key=rest;
  setProperty(key,value);
      }
    
    }
  }

  void RateMonotonicScheduler::setProperty(const char* key, const char* value){
  }

  bool RateMonotonicScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const std::map<int,ProcessControlBlock*> &ready_tasks,
    const  std::map<int,ProcessControlBlock*> &running_tasks)
  {
     return false;
  }
  /**
   *
   */  void RateMonotonicScheduler::addedNewTask(ProcessControlBlock *pcb){
    p_queue_entry pqe;
    pqe.fifo_order=order_counter++;
    pqe.pcb=pcb;
    pqueue.push(pqe);
  }
  /**
   *
   */  void RateMonotonicScheduler::removedTask(ProcessControlBlock *pcb){
  }

  /**
   *
   */
  scheduling_decision RateMonotonicScheduler::schedulingDecision(
    int& task_to_resign,
    int& task_to_assign,
    const  std::map<int,ProcessControlBlock*> &ready_tasks,
    const  std::map<int,ProcessControlBlock*> &running_tasks)
  {
    scheduling_decision ret_decision=ONLY_ASSIGN;
    if(pqueue.size()<=0) return NOCHANGE;  // kein neuer -> nichts tun
    p_queue_entry prior_ready=pqueue.top();// h�chste priorit�t der ready tasks

    // wert der priorit�t
    double d_prior_ready=prior_ready.pcb->getPriority();
    task_to_assign=prior_ready.pcb->getInstanceId();


    if(running_tasks.size()!=0){  // l�uft noch einer ?
      std::map<int,ProcessControlBlock*>::const_iterator iter;
      iter=running_tasks.begin();
      ProcessControlBlock *pcb=iter->second;

      //laufender mit h�herer oder gleicher priorit�t ->
      if(pcb->getPriority() <= d_prior_ready){
        ret_decision=NOCHANGE;                       //nicht verdr�ngen
      }else{
        ret_decision=PREEMPT;                        //verdr�ngen
        task_to_resign=pcb->getInstanceId(); 
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
