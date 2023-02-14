#include <systemcvpc/hscd_vpc_RateMonotonicScheduler.h>
#include <systemcvpc/hscd_vpc_Director.h>
#include <systemcvpc/hscd_vpc_Component.h>
#include <systemcvpc/hscd_vpc_datatypes.h>

namespace SystemC_VPC{
  RateMonotonicScheduler::RateMonotonicScheduler(const char *schedulername){

    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,rm_queue_compare>
      pqueue(comp);

    order_counter=0;
  }

  void RateMonotonicScheduler::setProperty(const char* key, const char* value){
  }

  bool RateMonotonicScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks)
  {
     return false;
  }
  /**
   *
   */  void RateMonotonicScheduler::addedNewTask(Task *task){
    p_queue_entry pqe;
    pqe.fifo_order=order_counter++;
    pqe.task=task;
    pqueue.push(pqe);
  }
  /**
   *
   */  void RateMonotonicScheduler::removedTask(Task *task){
  }

  /**
   *
   */
  scheduling_decision RateMonotonicScheduler::schedulingDecision(
    int& task_to_resign,
    int& task_to_assign,
    const  TaskMap &ready_tasks,
    const  TaskMap &running_tasks)
  {
    scheduling_decision ret_decision=ONLY_ASSIGN;
    if(pqueue.size()<=0) return NOCHANGE;  // kein neuer -> nichts tun
    p_queue_entry prior_ready=pqueue.top();// h�chste priorit�t der ready tasks

    // wert der priorit�t
    double d_prior_ready=prior_ready.task->getPriority();
    task_to_assign=prior_ready.task->getInstanceId();


    if(running_tasks.size()!=0){  // l�uft noch einer ?
      TaskMap::const_iterator iter;
      iter=running_tasks.begin();
      Task *task=iter->second;

      //laufender mit h�herer oder gleicher priorit�t ->
      if(task->getPriority() <= d_prior_ready){
        ret_decision=NOCHANGE;                       //nicht verdr�ngen
      }else{
        ret_decision=PREEMPT;                        //verdr�ngen
        task_to_resign=task->getInstanceId(); 
        pqueue.pop();
        p_queue_entry pqe={0,task};
        pqueue.push(pqe);
      }
    }else{
      pqueue.pop();
      ret_decision=ONLY_ASSIGN;  
    }
    
    
    return ret_decision;
  }
}
