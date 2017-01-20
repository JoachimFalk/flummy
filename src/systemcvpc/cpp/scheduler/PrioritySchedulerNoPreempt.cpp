/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <systemcvpc/Director.hpp>
#include <systemcvpc/datatypes.hpp>

#include "ComponentImpl.hpp"
#include "PrioritySchedulerNoPreempt.hpp"

namespace SystemC_VPC{

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
