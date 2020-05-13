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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_ROUNDROBINSCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_ROUNDROBINSCHEDULER_HPP

#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"

#include <systemc>

#include <map>
#include <deque>

namespace SystemC_VPC { namespace Detail {

  class PreemptiveComponent;

  class RoundRobinScheduler : public Scheduler{
  public:

    RoundRobinScheduler() :
      timeSlice_(10, sc_core::SC_NS), timeSliceExpires_()
    {
    }
    virtual ~RoundRobinScheduler(){}
    bool getSchedulerTimeSlice(sc_core::sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    void addedNewTask(TaskInstanceImpl *task);
    void removedTask(TaskInstanceImpl *task);

    //
    scheduling_decision
    schedulingDecision(int& task_to_resign,
                       int& task_to_assign,
                       const  TaskMap &ready_tasks,
                       const  TaskMap &running_tasks);
    void setProperty(const char* key, const char* value);
    sc_core::sc_time* schedulingOverhead();
    
  private:
    std::deque<int> rr_fifo;
    sc_core::sc_time timeSlice_;
    sc_core::sc_time timeSliceExpires_;

    int assignFromFront();
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_ROUNDROBINSCHEDULER_HPP */
