#include "hscd_vpc_PrioritySchedulerNoPreempt.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"
#include "hscd_vpc_datatypes.h"

namespace SystemC_VPC{
  PrioritySchedulerNoPreempt::PrioritySchedulerNoPreempt(const char *schedulername){

    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,p_queue_compare>
      pqueue(comp);

    order_counter=0;
  }

  void PrioritySchedulerNoPreempt::setProperty(const char* key, const char* value){
  }

  bool PrioritySchedulerNoPreempt::getSchedulerTimeSlice(
    sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {
    return false;
  }
  /**
   *
   */
  void PrioritySchedulerNoPreempt::addedNewTask(Task *task){
    p_queue_entry pqe;
    pqe.fifo_order=order_counter++;
    pqe.task=task;
    pqueue.push(pqe);
  }
  /**
   *
   */
  void PrioritySchedulerNoPreempt::removedTask(Task *task){
  }

  /**
   *
   */
   scheduling_decision PrioritySchedulerNoPreempt::schedulingDecision(
     int& task_to_resign,
     int& task_to_assign,
     const  TaskMap &ready_tasks,
     const  TaskMap &running_tasks)
   {
     scheduling_decision ret_decision=NOCHANGE;
     if(pqueue.size()<=0) return NOCHANGE;    // kein neuer -> nichts tun

     // hoechste prioritaet�der ready tasks
     p_queue_entry prior_ready=pqueue.top();
     task_to_assign=prior_ready.task->getInstanceId();

     if(running_tasks.size()!=0){
        //nothing to do, NO PREEMPTIVE-Scheduler
     }else{
       pqueue.pop();
       ret_decision=ONLY_ASSIGN;  
     }

     return ret_decision;
   }
}
