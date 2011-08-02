/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef HSCD_VPC_ABSTRACTCOMPONENT_H
#define HSCD_VPC_ABSTRACTCOMPONENT_H

#include <vector>
#include <map>

#include <systemc.h>

#include <string.h>

#include <CoSupport/SystemC/systemc_support.hpp>

#include <systemcvpc/vpc_config.h>

#include "config/Component.hpp"
#include "datatypes.hpp"
#include "Delayer.hpp"
#include "ProcessControlBlock.hpp"
#include "FunctionTimingPool.hpp"
#include "PCBPool.hpp"
#include "Task.hpp"
#include "PowerSumming.hpp"
#include "PowerMode.hpp"
#include "PluggablePowerGovernor.hpp"
#include "ComponentInfo.hpp"
#include "ComponentModel.hpp"
#include "Attribute.hpp"
#include "FunctionTimingPool.hpp"

namespace SystemC_VPC{

class ComponentObserver;

  using CoSupport::SystemC::Event;

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;
  typedef std::vector<ProcessId> ScheduledTasks;


  /**
   * \brief The interface of a Virtual-Processing-Component (VPC).
   */
  class AbstractComponent:
    public sc_module,
    public Delayer,
    public ComponentModel,
    public ComponentInterface
{
  
  public:

    virtual ~AbstractComponent(){
      this->timingPools.clear();
    }

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual bool setAttribute(AttributePtr attributePtr);

    /**
     * \brief Create the process control block.
     */
    ProcessControlBlockPtr createPCB(const ProcessId pid){
      assert(!hasPCB(pid));

      pcbPool[pid].reset(new ProcessControlBlock( this ));
      pcbPool[pid]->setPid(pid);

      return getPCB(pid);
    }

    bool hasPCB(const ProcessId pid) const
    {
      const PCBPool& pool = this->getPCBPool();
      return (pool.find(pid) != pool.end());
    }

    ProcessControlBlockPtr getPCB(const ProcessId pid) const
    {
      const PCBPool& pool = this->getPCBPool();
      assert(hasPCB(pid));
      return pool.find(pid)->second;
    }

    /**
     *
     */
    const PCBPool& getPCBPool() const {
      return this->pcbPool;
    }

    /**
     *
     */
    void addScheduledTask(ProcessId pid){
      if (scheduledTasks.empty() || scheduledTasks.back() != pid){
        scheduledTasks.push_back(pid);
      }
    }

    virtual void setDynamicPriority(std::list<ScheduledTask *> priorityList)
    {
      throw Config::ConfigException(std::string("Component ") + this->name() +
          " doesn't support dynamic priorities!");
    }

    virtual std::list<ScheduledTask *> getDynamicPriority()
    {
      throw Config::ConfigException(std::string("Component ") + this->name() +
          " doesn't support dynamic priorities!");
    }


    virtual void scheduleAfterTransition()
    {
      throw Config::ConfigException(std::string("Component ") + this->name() +
          " doesn't support scheduleAfterTransition()!");
    }

    virtual Trace::Tracing * getOrCreateTraceSignal(std::string name) = 0;

  protected:

    std::map<const PowerMode*, sc_time> transactionDelays;
    ScheduledTasks scheduledTasks;

  public:
  
    AbstractComponent(Config::Component::Ptr component) :
        sc_module(sc_module_name(component->getName().c_str())),
        Delayer(component->getComponentId(),
            component->getName()),
        transactionDelays(),
        scheduledTasks(),
        powerMode(NULL),
        localGovernorFactory(NULL),
        midPowerGov(NULL),
        powerAttribute(new Attribute("",""))
    {
      component->componentInterface_ = this;
      if(powerTables.find(getPowerMode()) == powerTables.end()){
        powerTables[getPowerMode()] = PowerTable();
      }

      PowerTable &powerTable=powerTables[getPowerMode()];
      powerTable[ComponentState::IDLE]    = 0.0;
      powerTable[ComponentState::RUNNING] = 1.0;
    }
            
    /**
       * \brief Simulate an execution on this "Virtual Component".
       *
       * While this simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(Task* task)=0;

    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker)=0;

    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker)=0;

    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker)=0;

    /**
     * 
     */
    virtual void updatePowerConsumption() = 0;

    /**
     * 
     */
    void setPowerMode(const PowerMode* mode);


    const PowerMode* getPowerMode() const {
      return this->powerMode;
    }

    /*
     * from ComponentInterface
     */
    void changePowerMode(std::string powerMode) {
      setPowerMode(translatePowerMode(powerMode));
    }

    /*
         * This function sets the appropriate execution state of the component according to the component powerstate
         * (component's power state info is not encapsulated here, so it is the responsability of the powerState object to call this
         * function whenever a powermode change takes place.
         *
         * Assumptions:
         * "Disabling" a component (i.e. SLEEPING execution state) will remember the previous state and come back to it
         * when leaving SLEEPING state)
         *
        */
    void forceComponentState(const PowerMode * powerMode);

    /**
     *
     */
    FunctionTimingPtr getTiming(const PowerMode *mode, ProcessId pid);

  private:

    bool processPower(AttributePtr att);

    void loadLocalGovernorPlugin(std::string plugin);


    /**
     *
     */
    PCBPool pcbPool;
    FunctionTimingPoolPtr timingPool;
    std::map<const PowerMode*, FunctionTimingPoolPtr> timingPools;
    const PowerMode *powerMode;

  protected:
    PlugInFactory<PluggableLocalPowerGovernor> *localGovernorFactory;
    PluggableLocalPowerGovernor *midPowerGov;
    AttributePtr powerAttribute;
    typedef std::map<std::string,
                     DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >* >
      Factories;
    static Factories factories;
    PowerTables powerTables;
    };
}

#endif //HSCD_VPC_ABSTRACTCOMPONENT_H
