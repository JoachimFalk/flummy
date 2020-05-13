// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_NONPREEMPTIVECOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_NONPREEMPTIVECOMPONENT_HPP

#include <systemc>

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Component.hpp>

#include "../AbstractComponent.hpp"
#include "../HysteresisLocalGovernor.hpp"
#include "../PowerSumming.hpp"
#include "../TaskInstanceImpl.hpp"
#include "../Director.hpp"
#include "../timetriggered/tt_support.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>

#include <vector>
#include <map>
#include <deque>
#include <queue>
#include <list>

namespace SystemC_VPC { namespace Detail {

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class NonPreemptiveComponent: public AbstractComponent {
    
    SC_HAS_PROCESS(NonPreemptiveComponent);

  public:
    /**
     * implementation of AbstractComponent::compute(Task *)
     */
    virtual void compute(TaskInstanceImpl* task);

    /**
     *
     */
    virtual void requestBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);
    
    /*
     * from ComponentInterface
     */
    bool hasWaitingOrRunningTasks();
      
  protected:
    /**
     * \brief An implementation of AbstractComponent used together with
     * passive actors and global SMoC v2 Schedulers.
     */
    NonPreemptiveComponent(std::string const &name);

    virtual void newReadyTask(TaskInstanceImpl *newTask) = 0;

    virtual TaskInstanceImpl *selectReadyTask() = 0;

    virtual ~NonPreemptiveComponent();

  private:
    // This is the set of active tasks that are signaled by notifyActivation.
    // Note that these tasks might not be ready due to
    // getNextReleaseTime() > sc_core::sc_time_stamp().
    std::set<TaskInterface *> activeTasks;

    // This is the queue for tasks arriving via notifyActivation but where
    // getNextReleaseTime() > sc_core::sc_time_stamp().
    TT::TimedQueue ttReleaseQueue;

    void ttReleaseQueueMethod();
    sc_core::sc_event ttReleaseQueueEvent;

    void addTask(TaskInstanceImpl *newTask);

    unsigned int  readyTasks;
    TaskInstanceImpl         *runningTask;

    void scheduleThread();
    sc_core::sc_event scheduleEvent;

    typedef CoSupport::SystemC::Event Event;

    Event blockCompute;
#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    InternalLoadHysteresisGovernor *midPowerGov;

    void removeTask();

    bool processPower(Attribute att);
    
    void notifyActivation(TaskInterface *scheduledTask,
        bool active);
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_NONPREEMPTIVECOMPONENT_HPP */
