#include <systemcvpc/PriorityScheduler.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/Component.hpp>
#include <systemcvpc/datatypes.hpp>

namespace SystemC_VPC{
  PriorityScheduler::PriorityScheduler(const char *schedulername){

    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,p_queue_compare>
      pqueue(comp);

    order_counter=0;
  }

  void PriorityScheduler::setProperty(const char* key, const char* value){
  }

  bool PriorityScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {
    return false;
  }
  /**
   *
   */
  void PriorityScheduler::addedNewTask(Task *task){
    p_queue_entry pqe;
    pqe.fifo_order=order_counter++;
    pqe.task=task;
    pqueue.push(pqe);
  }
  /**
   *
   */
  void PriorityScheduler::removedTask(Task *task){
  }

  /**
   *
   */
   scheduling_decision PriorityScheduler::schedulingDecision(
     int& task_to_resign,
     int& task_to_assign,
     const  TaskMap &ready_tasks,
     const  TaskMap &running_tasks)
   {
     scheduling_decision ret_decision=ONLY_ASSIGN;
     if(pqueue.size()<=0) return NOCHANGE;    // no new task -> no change

     // unassigned task with highest priority
     p_queue_entry prior_ready=pqueue.top();

     double d_prior_ready=prior_ready.task->getPriority();
     task_to_assign=prior_ready.task->getInstanceId();


     if(running_tasks.size()!=0){  // is another task running?
       TaskMap::const_iterator iter;
       iter=running_tasks.begin();
       Task *task=iter->second;

       //has running task higher priority (lesser value)
       if(task->getPriority() <= d_prior_ready){
         ret_decision=NOCHANGE;
       }else{
         ret_decision=PREEMPT;                        //preempt task
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
