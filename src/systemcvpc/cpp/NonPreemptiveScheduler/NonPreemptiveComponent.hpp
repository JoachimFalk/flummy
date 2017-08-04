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

#ifndef _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_NONPREEMPTIVECOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_NONPREEMPTIVECOMPONENT_HPP

#include <systemc>

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/config/Component.hpp>

#include "../AbstractComponent.hpp"
#include "../ComponentInfo.hpp"
#include "../HysteresisLocalGovernor.hpp"
#include "../PowerSumming.hpp"
#include "../PowerMode.hpp"
#include "../Task.hpp"
#include "../tracing/TracerIf.hpp"

#include <vector>
#include <map>
#include <deque>
#include <queue>
#include <list>

#include "../debug_config.hpp"
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_FCFSCOMPONENT
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "../debug_on.hpp"
#else
  #include "../debug_off.hpp"
#endif

namespace SystemC_VPC{

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class NonPreemptiveComponent : public AbstractComponent{
    
    SC_HAS_PROCESS(NonPreemptiveComponent);

  public:

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(Task* task);


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

    /*
     * from ComponentInterface
     */
    bool hasWaitingOrRunningTasks();

    /**
     * \brief An implementation of AbstractComponent used together with
     * passive actors and global SMoC v2 Schedulers.
     */
    NonPreemptiveComponent(Config::Component::Ptr component, Director *director);
      
    virtual ~NonPreemptiveComponent();
    
    void addPowerGovernor(PluggableLocalPowerGovernor *gov);

    virtual Trace::Tracing *getOrCreateTraceSignal(std::string name);

  protected:

    void schedule_method();

    void remainingPipelineStages();

    void fireStateChanged(const ComponentState &state);

    Task*                  runningTask;
    sc_core::sc_event notify_scheduler_thread;

    // time last task started
    sc_core::sc_time startTime;
  private:
    sc_core::sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair> pqueue;

    //PowerTables powerTables;
    
    Event blockCompute;
    size_t   blockMutex;
#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    InternalLoadHysteresisGovernor *midPowerGov;

    bool releasePhase;

    bool processPower(Attribute att);

    void moveToRemainingPipelineStages(Task *task);
    
    void setScheduler(const char *schedulername);

    void removeTask();

    virtual void addTask(Task *newTask) = 0;

    virtual Task *scheduleTask() = 0;

    virtual void notifyActivation(ScheduledTask * scheduledTask,
        bool active) = 0;

    virtual bool releaseActor() = 0;

    virtual bool hasReadyTask() = 0;
  };

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_NONPREEMPTIVECOMPONENT_HPP */
