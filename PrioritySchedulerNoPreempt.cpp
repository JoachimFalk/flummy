/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <systemcvpc/PrioritySchedulerNoPreempt.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/Component.hpp>
#include <systemcvpc/datatypes.hpp>

namespace SystemC_VPC{
  PrioritySchedulerNoPreempt::PrioritySchedulerNoPreempt(
    const char *schedulername) : pqueue()
  {
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
    p_queue_entry pqe(order_counter++, task);
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
