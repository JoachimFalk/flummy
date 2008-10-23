/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_AbstractComponent.h
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

#include "hscd_vpc_datatypes.h"
#include "Delayer.h"
#include "hscd_vpc_ProcessEventListener.h"
#include "hscd_vpc_ProcessControlBlock.h"
#include "FunctionTimingPool.h"
#include "PCBPool.h"
#include "Task.h"
#include "PowerMode.h"
#include "ComponentInfo.h"
#include "ComponentModel.h"
#include "Attribute.h"

namespace SystemC_VPC{

class ComponentObserver;

  using CoSupport::SystemC::Event;
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
    virtual void processAndForwardParameter(char *sType,char *sValue)=0;
    
    virtual void setAttribute(Attribute& fr_Attribute)=0;

    const char* getName() const;

    /**
     * \brief Create the process control block.
     */
    ProcessControlBlock& createPCB(ProcessId pid){
      PCBPool& pool = this->getPCBPool();
      if(pool.find(pid) == pool.end()){
        pool[pid] = new ProcessControlBlock( this );
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
    virtual void requestBlockingCompute(Task* task, Event* blocker)=0;

    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Event* blocker)=0;

    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Event* blocker)=0;

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
        timingPools[powerMode] = new FunctionTimingPool();
      }
      this->timingPool = timingPools[powerMode];
    }

    const PowerMode* getPowerMode() const {
      return this->powerMode;
    }

    /**
     *
     */
    FunctionTiming * getTiming(const PowerMode *mode, ProcessId pid){
      if(timingPools.find(mode) == timingPools.end()){
        timingPools[mode] = new FunctionTimingPool();
      }
      FunctionTimingPool * pool = this->timingPools[mode];
      if(pool->find(pid) == pool->end()){
        (*pool)[pid] = new FunctionTiming();
        (*pool)[pid]->setBaseDelay(this->transactionDelays[mode]);
      }
      return (*pool)[pid];
    }
    private:
    /**
     *
     */
    PCBPool pcbPool;
    FunctionTimingPool * timingPool;
    std::map<const PowerMode*, FunctionTimingPool*> timingPools;
    const PowerMode *powerMode;
  };
  
}

#endif //HSCD_VPC_ABSTRACTCOMPONENT_H
