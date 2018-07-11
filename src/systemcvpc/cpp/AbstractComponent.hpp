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

#ifndef _INCLUDED_SYSTEMCVPC_ABSTRACTCOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_ABSTRACTCOMPONENT_HPP

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Attribute.hpp>

#include "tracing/TraceableComponent.hpp"
#include "Delayer.hpp"
#include "FunctionTiming.hpp"
#include "TaskInstance.hpp"
#include "PowerSumming.hpp"
#include "PowerMode.hpp"
#include "PluggablePowerGovernor.hpp"
#include "ComponentInfo.hpp"
#include "ComponentModel.hpp"
#include "timetriggered/tt_support.hpp"
#include "ProcessControlBlock.hpp"

#include "config.h"

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>
#include <smoc/SimulatorAPI/TaskInterface.hpp>
#include <smoc/SimulatorAPI/FiringRuleInterface.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include <CoSupport/SystemC/systemc_support.hpp>

#include <systemc>

#include <boost/shared_ptr.hpp>

#include <list>
#include <vector>
#include <map>
#include <string>

namespace SystemC_VPC {

  class ComponentObserver;

  using CoSupport::SystemC::Event;

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;
  typedef std::string MultiCastGroup;


  /**
   * \brief The interface of a Virtual-Processing-Component (VPC).
   */
  class AbstractComponent:
    public sc_core::sc_module,
    public Tracing::TraceableComponent,
    public Delayer,
    private smoc::SimulatorAPI::SchedulerInterface,
    public ComponentModel,
    public Config::ComponentInterface
  {
  public:
    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual bool setAttribute(AttributePtr attributePtr);

    /**
     * \brief Create the Process Control Block (PCB).
     * The PCB must not previously exist.
     */
    ProcessControlBlock *createPCB(std::string const &taskName);

    void registerTask(TaskInterface *task);

    /**
     * \brief Get the Process Control Block (PCB) for pid.
     * The PCB must previously have been created via createPCB.
     */
    ProcessControlBlock *getPCB(ProcessId const pid) const;

    virtual void setDynamicPriority(std::list<ScheduledTask *> priorityList);
    virtual std::list<ScheduledTask *> getDynamicPriority();

    virtual void scheduleAfterTransition();

    void requestCanExecute();

    bool getCanExecuteTasks() const;

    void setCanExecuteTasks(bool canExecuteTasks);

    virtual void reactivateExecution();


    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute(TaskInstance* task)=0;

    /**
     *
     */
    virtual void requestBlockingCompute(TaskInstance* task, Coupling::VPCEvent::Ptr blocker)=0;

    /**
     *
     */
    virtual void execBlockingCompute(TaskInstance* task, Coupling::VPCEvent::Ptr blocker)=0;

    /**
     *
     */
    virtual void abortBlockingCompute(TaskInstance* task, Coupling::VPCEvent::Ptr blocker)=0;

    /**
     * 
     */
    virtual void updatePowerConsumption() = 0;

    /**
     * from ComponentInterface
     */
    void             setPowerMode(const PowerMode *mode);
    /**
     * from ComponentInterface
     */
    const PowerMode *getPowerMode() const;

    /*
     * from ComponentInterface
     */
    void changePowerMode(std::string powerMode);

    /*
     * from ComponentInterface
     */
    void setCanExec(bool canExec);

    /*
     * from ComponentInterface
     */
    void registerComponentWakeup(const ScheduledTask * actor, Coupling::VPCEvent::Ptr event);

    /*
     * from ComponentInterface
     */
    void registerComponentIdle(const ScheduledTask * actor, Coupling::VPCEvent::Ptr event);

    /**
     *
     */
    FunctionTimingPtr getTiming(const PowerMode *mode, ProcessId pid);

    TaskInstance *executeHop(ProcessControlBlock *pcb, size_t quantum, EventPair const &np);
  protected:
    void end_of_elaboration();

    /// Called once per actor firing to indicate that the DII of the task instance is over.
    void finishDiiTaskInstance(TaskInstance *taskInstance);

    /// Called once per actor firing to indicate that the latency of the task instance is over.
    void finishLatencyTaskInstance(TaskInstance *taskInstance);

    typedef boost::shared_ptr<ProcessControlBlock>        ProcessControlBlockPtr;
    typedef std::map<ProcessId, ProcessControlBlockPtr>   PCBPool;

    std::map<const PowerMode*, sc_core::sc_time> transactionDelays;
    bool requestExecuteTasks;
    std::map<ProcessId, MultiCastGroup> multiCastGroups;

    struct MultiCastGroupInstance{
      MultiCastGroup mcg;
      sc_core::sc_time timestamp;
      TaskInstance* task;
      std::list<TaskInstance*>* additional_tasks;
    };

    std::list<MultiCastGroupInstance*> multiCastGroupInstances;

    PlugInFactory<PluggableLocalPowerGovernor> *localGovernorFactory;
    PluggableLocalPowerGovernor *midPowerGov;
    AttributePtr powerAttribute;
    typedef std::map<std::string,
                     DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >* >
      Factories;
    static Factories factories;
    PowerTables powerTables;

    AbstractComponent(Config::Component::Ptr component);

    MultiCastGroupInstance* getMultiCastGroupInstance(TaskInstance* actualTask);

    /**
     *
     */
    const PCBPool& getPCBPool() const {
      return this->pcbPool;
    }

    bool requestShutdown();

    virtual ~AbstractComponent();
  private:
    class InputsAvailableListener;
    class OutputWrittenListener;

    /// Implement interface to SysteMoC
    void checkFiringRule(TaskInterface *task, smoc::SimulatorAPI::FiringRuleInterface *fr);

    /// Implement interface to SysteMoC
    void registerFiringRule(TaskInterface *task, smoc::SimulatorAPI::FiringRuleInterface *fr);

    /// Implement interface to SysteMoC
    void executeFiringRule(TaskInterface *task, smoc::SimulatorAPI::FiringRuleInterface *fr);

    bool processPower(AttributePtr att);

    /**
     * process attributes/parameters for MultiCast Configuration
     */
    bool processMCG(AttributePtr attribute);

    void loadLocalGovernorPlugin(std::string plugin);

    PCBPool pcbPool;
    FunctionTimingPoolPtr timingPool;
    std::map<const PowerMode*, FunctionTimingPoolPtr> timingPools;
    const PowerMode *powerMode;
    bool canExecuteTasks;
    sc_core::sc_time shutdownRequestAtTime;
    Coupling::VPCEvent::Ptr componentWakeup;
    Coupling::VPCEvent::Ptr componentIdle;
  };

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_ABSTRACTCOMPONENT_HPP */
