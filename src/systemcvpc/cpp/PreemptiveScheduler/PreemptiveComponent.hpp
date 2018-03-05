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

#ifndef _INCLUDED_SYSTEMCVPC_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/config/Scheduler.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>

#include "Scheduler.hpp"
#include "../AbstractComponent.hpp"
#include "../ComponentInfo.hpp"
#include "../PowerMode.hpp"
#include "../PowerSumming.hpp"
#include "../timetriggered/tt_support.hpp"
#include "../tracing/TracerIf.hpp"

#include <systemc>

#include <vector>
#include <map>
#include <deque>
#include <queue>

namespace SystemC_VPC {

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class PreemptiveComponent: public AbstractComponent {
    SC_HAS_PROCESS(PreemptiveComponent);
  public:
    /**
     * \brief An implementation of AbstractComponent.
     */
    PreemptiveComponent( Config::Component::Ptr component);

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(Task* task);

    /**
     *
     */
    bool setAttribute(AttributePtr attribute);

    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void updatePowerConsumption();

    /**
     *
     */
    virtual Trace::Tracing *getOrCreateTraceSignal(std::string name);

    /*
     * from ComponentInterface
     */
    bool hasWaitingOrRunningTasks();
      
    void addPowerGovernor(PluggableLocalPowerGovernor * gov){
      this->addObserver(gov);
    }

    void reactivateExecution();

    void notifyActivation(TaskInterface * scheduledTask,
        bool active);

    bool addStream(ProcessId pid);

    bool closeStream(ProcessId pid);

    void addTasks();

    virtual ~PreemptiveComponent();
  protected:
    virtual void schedule_thread();
    virtual void remainingPipelineStages();
    virtual void moveToRemainingPipelineStages(Task *task);

    sc_core::sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair> pqueue;
    bool pendingTask;

    Scheduler *scheduler;
    std::deque<Task*>      newTasks;
    std::deque<Task*>      disabledTasks;
    sc_core::sc_event notify_scheduler_thread;
    Event blockCompute;
    size_t   blockMutex;
    unsigned int max_used_buffer;
    unsigned int max_avail_buffer;

    // time last task started
    sc_core::sc_time startTime;

    void fireStateChanged(const ComponentState &state);

    const TaskMap &getReadyTasks()
      { return readyTasks; }

    const TaskMap &getRunningTasks()
      { return runningTasks; }

    TaskMap readyTasks;
    TaskMap runningTasks;

  private:
    sc_core::sc_event releaseActors;
    TT::TimedQueue ttReleaseQueue;

#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    void initialize(const Director* d);

    void setScheduler(Config::Component::Ptr component);

    void releaseActorsMethod();
  };

} 

#endif /* _INCLUDED_SYSTEMCVPC_PREEMPTIVESCHEDULER_PREEMPTIVECOMPONENT_HPP */
