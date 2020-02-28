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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/Scheduler.hpp>
#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"
#include "../AbstractComponent.hpp"
#include "../PowerSumming.hpp"
#include "../Director.hpp"
#include "../timetriggered/tt_support.hpp"

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
    void addAttribute(Attribute::Ptr attribute);

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

    void initialize(const Director* d);

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

    void remainingPipelineStages();
    void moveToRemainingPipelineStages(TaskInstanceImpl *task);

    sc_core::sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair> pqueue;

    Scheduler *scheduler;

    Event    blockCompute;
    size_t   blockMutex;

    TaskMap readyTasks;
    TaskMap runningTasks;

#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP */
