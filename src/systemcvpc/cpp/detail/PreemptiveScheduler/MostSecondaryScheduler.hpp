// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSECONDARYSCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSECONDARYSCHEDULER_HPP

#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"

#include <systemc>

#include <map>
#include <deque>

namespace SystemC_VPC { namespace Detail {

  class PreemptiveComponent;

  struct AsynchSlot{
      sc_core::sc_time length;
      int process;
      int Id;
      int priority;
      std::string name;
    };

class MostSecondaryScheduler :public Scheduler{
public:
  MostSecondaryScheduler(){
    sysFreq = 48000;
  }


  sc_core::sc_time cycle(int sysFreq);

  void addedNewTask(TaskInstanceImpl *task);

  void removedTask(TaskInstanceImpl *task);

  bool getSchedulerTimeSlice(sc_core::sc_time& time,
                              const TaskMap &ready_tasks,
                              const TaskMap &running_tasks);

  scheduling_decision schedulingDecision(
       int& task_to_resign,
       int& task_to_assign,
       const  TaskMap &ready_tasks,
       const  TaskMap &running_tasks);

  sc_core::sc_time* schedulingOverhead(){return 0;}


private:

std::deque<AsynchSlot> Asynch_slots;
sc_core::sc_time lastassignasynch;
int sysFreq;


};

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSECONDARYSCHEDULER_HPP */
