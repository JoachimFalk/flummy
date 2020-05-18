// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
 *   2013 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/Scheduler.hpp>
#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"
#include "../AbstractComponent.hpp"
#include "../timetriggered/tt_support.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>

#include <systemc>

#include <vector>
#include <map>
#include <set>
#include <deque>
#include <queue>

namespace SystemC_VPC { namespace Detail {

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class PreemptiveComponent: public AbstractComponent {
    SC_HAS_PROCESS(PreemptiveComponent);

    typedef AbstractComponent base_type;
  public:
    /**
     * \brief An implementation of AbstractComponent.
     */
    PreemptiveComponent(std::string const &name, Scheduler *scheduler);

    /**
     * implementation of AbstractComponent::compute(Task *)
     */
    void compute(TaskInstanceImpl* task);

    /**
     *
     */
    void addAttribute(Attribute const &attribute);

    /**
     *
     */
    void requestBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);
    
    /**
     *
     */
    void execBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);
    
    /**
     *
     */
    void abortBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);
    
    /*
     * from ComponentInterface
     */
    bool hasWaitingOrRunningTasks();
      
    void reactivateExecution();

    void notifyActivation(TaskInterface * scheduledTask,
        bool active);

    bool addStream(ProcessId pid);

    bool closeStream(ProcessId pid);

    void finalize();

    virtual ~PreemptiveComponent();
  private:
    // This is the set of active tasks that are signaled by notifyActivation.
    // Note that these tasks might not be ready due to
    // actor->getNextReleaseTime() > sc_core::sc_time_stamp() or
    // !this->getCanExecuteTasks()
    std::set<TaskInterface *> activeTasks;

    // This is the queue for tasks arriving via notifyActivation but where
    // actor->getNextReleaseTime() > sc_core::sc_time_stamp() or
    // !this->getCanExecuteTasks()
    TT::TimedQueue ttReleaseQueue;

    void ttReleaseQueueMethod();
    sc_core::sc_event ttReleaseQueueEvent;

    // This is the queue for PSM tasks arriving via notifyActivation but where
    // actor->getNextReleaseTime() > sc_core::sc_time_stamp().
    TT::TimedQueue ttReleaseQueuePSM;

    void ttReleaseQueuePSMMethod();
    sc_core::sc_event ttReleaseQueuePSMEvent;

    void addTask(TaskInstanceImpl *newTask);

    void scheduleThread();
    sc_core::sc_event scheduleEvent;

    void removeTask(TaskInstanceImpl *removedTask);

    typedef CoSupport::SystemC::Event Event;

    Scheduler *scheduler;

    Event    blockCompute;
    size_t   blockMutex;

    TaskMap readyTasks;
    TaskMap runningTasks;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP */
