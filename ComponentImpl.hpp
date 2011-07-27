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

#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include <systemc.h>

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ComponentInfo.hpp>
#include <systemcvpc/config/Scheduler.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/PowerMode.hpp>
#include <systemcvpc/PowerSumming.hpp>
#include <systemcvpc/vpc_config.h>

#include "timetriggered/tt_support.hpp"

#include <vector>
#include <map>
#include <deque>
#include <queue>

namespace SystemC_VPC{

  class Scheduler;

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
    bool setAttribute(AttributePtr attribute);

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
    Component( Config::Component::Ptr component)
      : AbstractComponent(component),
        blockMutex(0)
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      SC_METHOD(releaseActorsMethod);
      dont_initialize();
      sensitive << releaseActors;
      this->setPowerMode(this->translatePowerMode("SLOW"));
      setScheduler(component);

#ifndef NO_POWER_SUM
      std::string powerSumFileName(this->getName());
      powerSumFileName += ".dat";

      powerSumStream = new std::ofstream(powerSumFileName.c_str());
      powerSumming   = new PowerSumming(*powerSumStream);
      this->addObserver(powerSumming);
#endif // NO_POWER_SUM
    }
      
    virtual ~Component();
    
    void addPowerGovernor(PluggableLocalPowerGovernor * gov){
      this->addObserver(gov);
    }

    void notifyActivation(ScheduledTask * scheduledTask,
        bool active);

  protected:

    virtual void schedule_thread(); 

    virtual void remainingPipelineStages(); 

  private:
    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair> pqueue;

    Scheduler *scheduler;
    std::deque<Task*>      newTasks;
    
    sc_event notify_scheduler_thread;
    Event blockCompute;
    size_t   blockMutex;
    sc_event releaseActors;
    TT::TimedQueue ttReleaseQueue;

#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    void initialize(const Director* d);

    // time last task started
    sc_time startTime;

    void moveToRemainingPipelineStages(Task* task);
    
    void setScheduler(Config::Component::Ptr component);
    
    void fireStateChanged(const ComponentState &state);

    void addTasks();

    void releaseActorsMethod();
  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
