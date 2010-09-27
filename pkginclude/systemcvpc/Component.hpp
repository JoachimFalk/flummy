/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * Component.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include <systemc.h>

#include <systemcvpc/vpc_config.h>
#include "datatypes.hpp"
#include "AbstractComponent.hpp"
#include "ComponentInfo.hpp"
#include "PowerSumming.hpp"
#include "PowerMode.hpp"
#include "Director.hpp"

#include <vector>
#include <map>
#include <deque>
#include <queue>

namespace SystemC_VPC{

  class Scheduler;

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;

  /**
   * \brief An implementation of AbstractComponent.
   * 
   */
  class Component : public AbstractComponent{
    
    SC_HAS_PROCESS(Component);

  public:

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(Task* task);


    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, RefCountEventPtr blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(Task* task, RefCountEventPtr blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, RefCountEventPtr blocker);
    
    /**
     *
     */
    virtual void updatePowerConsumption();
      
    /**
     * \brief An implementation of AbstractComponent used together with
     * passive actors and global SMoC v2 Schedulers.
     */
    Component( std::string name,
               std::string schedulername,
               Director *director )
      : AbstractComponent(name.c_str()),
        blockMutex(0),
        localGovernorFactory(NULL),
        midPowerGov(NULL),
        powerAttribute(new Attribute("",""))
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      this->setPowerMode(this->translatePowerMode("SLOW"));
      setScheduler(schedulername.c_str());

      if(powerTables.find(getPowerMode()) == powerTables.end()){
        powerTables[getPowerMode()] = PowerTable();
      }

      PowerTable &powerTable=powerTables[getPowerMode()];
      powerTable[ComponentState::IDLE]    = 0.0;
      powerTable[ComponentState::RUNNING] = 1.0;

#ifndef NO_POWER_SUM
      std::string powerSumFileName(this->getName());
      powerSumFileName += ".dat";

      powerSumStream = new std::ofstream(powerSumFileName.c_str());
      powerSumming   = new PowerSumming(*powerSumStream);
      this->addObserver(powerSumming);
#endif // NO_POWER_SUM
    }
      
    virtual ~Component();

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void setAttribute(AttributePtr attribute);
    
    void addPowerGovernor(PluggableLocalPowerGovernor * gov){
      this->addObserver(gov);
    }

  protected:

    virtual void schedule_thread(); 

    virtual void remainingPipelineStages(); 

  private:
    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair, std::vector<timePcbPair>,timeCompare> pqueue;

    Scheduler *scheduler;
    std::deque<Task*>      newTasks;
    
    PowerTables powerTables;
    
    sc_event notify_scheduler_thread;
    Event blockCompute;
    size_t   blockMutex;
#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    PlugInFactory<PluggableLocalPowerGovernor> *localGovernorFactory;
    PluggableLocalPowerGovernor *midPowerGov;
    AttributePtr powerAttribute;
    typedef std::map<std::string,
                     DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >* >
      Factories;
    static Factories factories;
    

    bool processPower(AttributePtr att);

    void initialize(const Director* d);

    // time last task started
    sc_time startTime;

    void moveToRemainingPipelineStages(Task* task);
    
    void setScheduler(const char *schedulername);
    
    void fireStateChanged(const ComponentState &state);

    void addTasks();

    void loadLocalGovernorPlugin(std::string plugin);
  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
