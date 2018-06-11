// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SYSTEMCVPC_PREEMPTIVESCHEDULER_RATEMONOTONICSCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_PREEMPTIVESCHEDULER_RATEMONOTONICSCHEDULER_HPP

#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"

#include <systemc>

#include <map>
#include <queue>
#include <vector>

namespace SystemC_VPC{
  class PreemptiveComponent;

  struct rm_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
        const p_queue_entry& pqe2) const
    {
      double p1 = sc_core::sc_time(1,sc_core::SC_NS)/pqe1.task->getPeriod();
      double p2 = sc_core::sc_time(1,sc_core::SC_NS)/pqe2.task->getPeriod();
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (pqe1.fifo_order>pqe2.fifo_order);
      else
        return false;
    }

  };

  class RateMonotonicScheduler : public Scheduler{
  public:

    RateMonotonicScheduler() : order_counter(0) {
      std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,rm_queue_compare>
        pqueue(comp);
    }
    virtual ~RateMonotonicScheduler(){}
    bool getSchedulerTimeSlice(sc_core::sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    void addedNewTask(TaskInstance *task);
    void removedTask(TaskInstance *task);
    sc_core::sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks,
                                           const  TaskMap &running_tasks);
    void setProperty(const char* key, const char* value);
    sc_core::sc_time* schedulingOverhead(){return 0;}//;
  protected:
    int order_counter;
    rm_queue_compare comp;
    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,rm_queue_compare> pqueue;


  };
}
#endif /* _INCLUDED_SYSTEMCVPC_PREEMPTIVESCHEDULER_RATEMONOTONICSCHEDULER_HPP */
