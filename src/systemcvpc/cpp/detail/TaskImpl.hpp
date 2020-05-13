// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_TASKIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_TASKIMPL_HPP

#include <systemcvpc/EventPair.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Timing.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/Extending/Task.hpp>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include <systemc>

#include <vector>
#include <string>

namespace SystemC_VPC { namespace Detail {

  class AbstractComponent;
  class ObservableComponent;

 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class TaskImpl
    : public Extending::Task
  {
  public:

    ProcessId getPid() const
      { return pid; }

    void      setPriority(int priority)
      { this->priority = priority; }
    int       getPriority() const
      { return priority; }

    sc_core::sc_time getPeriod() const;

    void setTaskIsPSM(bool flag)
      { psm = flag; }
    bool getTaskIsPSM() const
      { return psm; }

//  void setScheduledTask(TaskInterface * st)
//    { this->scheduledTask = st; }
    TaskInterface * getScheduledTask()
      { return this->scheduledTask; }
    bool hasScheduledTask() const
      { return this->scheduledTask != NULL; }

  private:
    friend class ObservableComponent; // To access constructor and destructor.

    /**
     * \brief Initialize a newly created instance of TaskImpl.
     */
    TaskImpl(TaskInterface     *taskInterface);
    TaskImpl(std::string const &taskName);

    ~TaskImpl();

    TaskInterface *scheduledTask;
    ProcessId      pid;
    int            priority;
    bool           psm;
  };

  static inline
  TaskImpl *getTaskOfTaskInterface(TaskInterface const *task)
    { return reinterpret_cast<TaskImpl *>(task->getSchedulerInfo()); }

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_TASKIMPL_HPP */
