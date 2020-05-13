// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_STREAMSHAPERSCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_STREAMSHAPERSCHEDULER_HPP

#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"

#include <systemc>

#include <map>
#include <deque>

namespace SystemC_VPC { namespace Detail {

  class PreemptiveComponent;

  typedef size_t ProcessId;
  typedef std::deque< std::pair <std::string, std::string> > Properties;

  class StreamShaperScheduler : public Scheduler{
  public:
    
    StreamShaperScheduler();
    
    virtual ~StreamShaperScheduler(){}
    
    bool getSchedulerTimeSlice(sc_core::sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    
    void addedNewTask(TaskInstanceImpl *task);
    
    void removedTask(TaskInstanceImpl *task);
    
    void setAttribute(Attribute const &attribute);

    sc_core::sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks
                                           ,const  TaskMap &running_tasks);
    
    void setProperty(const char* key, const char* value);
    
    sc_core::sc_time* schedulingOverhead();
    
    void initialize();
    
  private:
    void _setProperty(const char* key, const char* value);
    
    sc_core::sc_time shapeCycle;
    bool firstrun;
    sc_core::sc_time lastassign;
    sc_core::sc_time remainingSlice;
    std::deque<std::pair<std::string, std::string> > _properties;
    std::deque<int> stream_fifo;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_STREAMSHAPERSCHEDULER_HPP */
