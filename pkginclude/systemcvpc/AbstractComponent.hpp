/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * AbstractComponent.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_ABSTRACTCOMPONENT_H
#define HSCD_VPC_ABSTRACTCOMPONENT_H

#include <vector>

#include <systemc.h>

#include <string.h>

#include <CoSupport/SystemC/systemc_support.hpp>

#include "datatypes.hpp"
#include "Delayer.hpp"
#include "ProcessEventListener.hpp"
#include "ProcessControlBlock.hpp"
#include "FunctionTimingPool.hpp"
#include "PCBPool.hpp"
#include "Task.hpp"
#include "PowerMode.hpp"
#include "ComponentInfo.hpp"
#include "ComponentModel.hpp"
#include "Attribute.hpp"
#include "FunctionTimingPool.hpp"

namespace SystemC_VPC{

class ComponentObserver;

  using CoSupport::SystemC::Event;
  using CoSupport::SystemC::RefCountEventPtr;
  /**
   * \brief The interface definition to a Virtual-Processing-Component (VPC).
   * 
   * An application using this Framework should call the AbstractComponent::compute(const char *, const char *, sc_event) Funktion.
   */
  class AbstractComponent:
    public sc_module, public Delayer, public ComponentModel {
  
  public:

    virtual ~AbstractComponent(){
      this->timingPools.clear();
    }

    /**
     * \brief Used to create the Tracefiles.
     *
     * To create a vcd-trace-file in SystemC all the signals to 
     * trace have to be in a "global" scope. The signals have to 
     * be created in elaboration phase (before first sc_start).
     */
    virtual void informAboutMapping(std::string module)=0;

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void setAttribute(AttributePtr attributePtr)=0;

    const char* getName() const;

    /**
     * \brief Create the process control block.
     */
    ProcessControlBlock& createPCB(ProcessId pid){
      PCBPool& pool = this->getPCBPool();
      if(pool.find(pid) == pool.end()){
        pool[pid].reset(new ProcessControlBlock( this ));
        pool[pid]->setPid(pid);
      }
      return *(pool[pid]);
    }

    /**
     *
     */
    PCBPool& getPCBPool(){
      return this->pcbPool;
    }
  protected:

    // points to direct associated controlling instance
    ProcessEventListener* parentControlUnit;
    std::map<const PowerMode*, sc_time> transactionDelays;
  
  public:
  
    AbstractComponent(sc_module_name name)
      : sc_module(name),
        Delayer(),
      powerMode(NULL) {}
            
    /**
       * \brief Simulate an execution on this "Virtual Component".
       *
       * While this simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(Task* task)=0;

    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, RefCountEventPtr blocker)=0;

    /**
     *
     */
    virtual void execBlockingCompute(Task* task, RefCountEventPtr blocker)=0;

    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, RefCountEventPtr blocker)=0;

    /**
     * \brief Sets next controlling instance of component
     * Used for callback mechanism to store pointer to "direct" controlling instance
     * for later infroming about finished or killed tasks.
     * \param controller points to controlling instance which is
     * responsible for component.
     */
    virtual void setParentController(ProcessEventListener* controller){
    
      this->parentControlUnit = controller;
    
    }
    
    /**
     * \brief Notifies parent controlling instance about task event
     * This mehtod is used to inform "direct" controlling instance about
     * finished or killed tasks.
     * \param task points to the finished or killed task
     */
    virtual void notifyParentController(Task* task){
      this->parentControlUnit->signalProcessEvent(task);
    }

    /**
     * 
     */
    virtual void updatePowerConsumption() = 0;

    /**
     * 
     */
    void setPowerMode(const PowerMode* mode){
      this->powerMode = translatePowerMode(mode->getName());
      this->updatePowerConsumption();

      if(timingPools.find(powerMode) == timingPools.end()){
        timingPools[powerMode].reset(new FunctionTimingPool());
      }
      this->timingPool = timingPools[powerMode];
    }

    const PowerMode* getPowerMode() const {
      return this->powerMode;
    }

    /**
     *
     */
    FunctionTimingPtr getTiming(const PowerMode *mode, ProcessId pid){
      if(timingPools.find(mode) == timingPools.end()){
        timingPools[mode].reset(new FunctionTimingPool());
      }
      FunctionTimingPoolPtr pool = this->timingPools[mode];
      if(pool->find(pid) == pool->end()){
        (*pool)[pid].reset(new FunctionTiming());
        (*pool)[pid]->setBaseDelay(this->transactionDelays[mode]);
      }
      return (*pool)[pid];
    }
    private:
    /**
     *
     */
    PCBPool pcbPool;
    FunctionTimingPoolPtr timingPool;
    std::map<const PowerMode*, FunctionTimingPoolPtr> timingPools;
    const PowerMode *powerMode;
  };
  
}

#endif //HSCD_VPC_ABSTRACTCOMPONENT_H
