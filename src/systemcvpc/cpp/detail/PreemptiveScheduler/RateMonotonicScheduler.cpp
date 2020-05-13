// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <systemcvpc/datatypes.hpp>

#include "RateMonotonicScheduler.hpp"
#include "PreemptiveComponent.hpp"

namespace SystemC_VPC { namespace Detail {

  void RateMonotonicScheduler::setProperty(const char* key, const char* value){
  }

  bool RateMonotonicScheduler::getSchedulerTimeSlice(
    sc_core::sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks)
  {
     return false;
  }
  /**
   *
   */  void RateMonotonicScheduler::addedNewTask(TaskInstanceImpl *task){
    p_queue_entry pqe(order_counter++, task);
    pqueue.push(pqe);
  }
  /**
   *
   */  void RateMonotonicScheduler::removedTask(TaskInstanceImpl *task){
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
    if(pqueue.size()<=0) return NOCHANGE;    // no new task -> no change

    // unassigned task with highest priority
    p_queue_entry prior_ready=pqueue.top();

    double d_prior_ready=prior_ready.task->getPriority();
    task_to_assign=prior_ready.task->getInstanceId();


    if(running_tasks.size()!=0){  // is another task running?
      TaskMap::const_iterator iter;
      iter=running_tasks.begin();
      TaskInstanceImpl *task=iter->second;

      //has running task higher priority (lesser value)
      if(task->getPriority() <= d_prior_ready){
        ret_decision=NOCHANGE;
      }else{
        ret_decision=PREEMPT;                        //preempt task
        task_to_resign=task->getInstanceId(); 
        pqueue.pop();
        p_queue_entry pqe(0,task);
        pqueue.push(pqe);
      }
    }else{
      pqueue.pop();
      ret_decision=ONLY_ASSIGN;  
    }
    
    
    return ret_decision;
  }

} } // namespace SystemC_VPC::Detail
