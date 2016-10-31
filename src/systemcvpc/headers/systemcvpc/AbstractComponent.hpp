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
#include "ScheduledTask.hpp"
#include "timetriggered/tt_support.hpp"
#include <list>

namespace SystemC_VPC{

class ComponentObserver;

  using CoSupport::SystemC::Event;

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;
  typedef std::vector<ProcessId> ScheduledTasks;
  typedef std::string MultiCastGroup;


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

    void requestCanExecute(){

      //assert(this->canExecuteTasks == false);
      componentIdle->reset();
      if(!requestExecuteTasks && componentWakeup != 0){
        //std::cout<< "Comp: " << this->getName()<<" requestCanExecute() - componentWakeup->notify() @ " << sc_time_stamp() <<  std::endl;
        //First request
        requestExecuteTasks=true;
        componentWakeup->notify();
      }
    }

    bool requestShutdown(){

        //FIXME: why did I use sc_pending_activity_at_current_time() here? what special-case?
      if(!hasWaitingOrRunningTasks() && (shutdownRequestAtTime == sc_time_stamp()) /*&& !sc_pending_activity_at_current_time()*/){
        if(componentIdle != 0){
          //std::cout<< "Comp: " << this->getName()<<" requestShutdown() - componentIdle->notify() @ " << sc_time_stamp() << " hasWaitingOrRunningTasks=" << hasWaitingOrRunningTasks()<< " " << sc_pending_activity_at_current_time() /*<< " " << m_simcontext->next_time()*/ <<  std::endl;
          //TODO: maybe notify it in the future?
          componentIdle->notify();
          if(sc_pending_activity_at_current_time()){
              return false;
          }
        }
      }else{
        shutdownRequestAtTime = sc_time_stamp();
        return false;
      }
      return true;
    }

    bool getCanExecuteTasks() const
    {
        return canExecuteTasks;
    }

    void setCanExecuteTasks(bool canExecuteTasks)
    {
      bool oldCanExecuteTasks = this->canExecuteTasks;
      this->canExecuteTasks = canExecuteTasks;
      if(!oldCanExecuteTasks && this->canExecuteTasks){
        requestExecuteTasks = false;
        this->reactivateExecution();
      }
    }

    virtual void reactivateExecution(){};

  protected:

    std::map<const PowerMode*, sc_time> transactionDelays;
    ScheduledTasks scheduledTasks;
    std::list<TT::TimeNodePair> tasksDuringNoExecutionPhase;
    bool requestExecuteTasks;
    std::map<ProcessId, MultiCastGroup> multiCastGroups;

    struct MultiCastGroupInstance{
      MultiCastGroup mcg;
      sc_time timestamp;
      Task* task;
      std::list<Task*>* additional_tasks;
    };

    std::list<MultiCastGroupInstance*> multiCastGroupInstances;


    MultiCastGroupInstance* getMultiCastGroupInstance(Task* actualTask){
      if(multiCastGroupInstances.size()!=0 ){
        //there are MultiCastGroupInstances, let's find the correct one
        for(std::list<MultiCastGroupInstance*>::iterator list_iter = multiCastGroupInstances.begin();
            list_iter != multiCastGroupInstances.end(); list_iter++)
        {
              MultiCastGroupInstance* mcgi = *list_iter;
            if(mcgi->mcg == multiCastGroups[actualTask->getProcessId()]){
              bool existing =  (mcgi->task->getProcessId() == actualTask->getProcessId());
              for(std::list<Task*>::iterator tasks_iter = mcgi->additional_tasks->begin();
                  tasks_iter != mcgi->additional_tasks->end(); tasks_iter++){
                  Task* task = *tasks_iter;
                  if(task->getProcessId() == actualTask->getProcessId()){
                      existing = true;
                  }
              }
              //we assume a fixed order of token-events, thus, the first free one is the correct one.
              if(!existing){
                  mcgi->additional_tasks->push_back(actualTask);
                  assert(mcgi->timestamp == sc_time_stamp()); // if not, MultiCastMessage reached at different times...
                  return mcgi;
              }
            }
        }
      }
      // no Instance found, create new one
      MultiCastGroupInstance* newInstance = new MultiCastGroupInstance();
      newInstance->mcg = multiCastGroups[actualTask->getProcessId()];
      newInstance->timestamp = sc_time_stamp();
      newInstance->task = actualTask;
      newInstance->additional_tasks = new  std::list<Task*>();
      multiCastGroupInstances.push_back(newInstance);
      return newInstance;
    }

  public:
  
    AbstractComponent(Config::Component::Ptr component) :
        sc_module(sc_module_name(component->getName().c_str())),
        Delayer(component->getComponentId(),
            component->getName()),
        transactionDelays(),
        scheduledTasks(),
        powerMode(NULL),
        canExecuteTasks(true),
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
     * from ComponentInterface
     */
    void setCanExec(bool canExec){
      this->setCanExecuteTasks(canExec);
    }

    /*
     * from ComponentInterface
     */
    void registerComponentWakeup(const ScheduledTask * actor, Coupling::VPCEvent::Ptr event){
      componentWakeup = event;
     }

    /*
     * from ComponentInterface
     */
    void registerComponentIdle(const ScheduledTask * actor, Coupling::VPCEvent::Ptr event){
      //std::cout<<"registerComponentIdle" << std::endl;
      componentIdle = event;
     }

    /*
     * from ComponentInterface
     */
    bool hasWaitingOrRunningTasks(){
      //std::cout<<"hasWaitingOrRunningTasks() " << readyTasks.size() <<" " <<  runningTasks.size() << " " << tasksDuringNoExecutionPhase.size() << std::endl;
      return (readyTasks.size() + runningTasks.size() + tasksDuringNoExecutionPhase.size()) > 0;
    }

        /* This function sets the appropriate execution state of the component according to the component powerstate
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

    /**
     * process attributes/parameters for MultiCast Configuration
     */
    bool processMCG(AttributePtr attribute);

    void loadLocalGovernorPlugin(std::string plugin);


    /**
     *
     */
    PCBPool pcbPool;
    FunctionTimingPoolPtr timingPool;
    std::map<const PowerMode*, FunctionTimingPoolPtr> timingPools;
    const PowerMode *powerMode;
    bool canExecuteTasks;
    sc_time shutdownRequestAtTime;
    Coupling::VPCEvent::Ptr componentWakeup;
    Coupling::VPCEvent::Ptr componentIdle;

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
