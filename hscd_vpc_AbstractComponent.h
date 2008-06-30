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

#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_ProcessEventListener.h"
#include "hscd_vpc_PCBPool.h"
#include "Task.h"
#include "PowerMode.h"
#include "ComponentInfo.h"

namespace SystemC_VPC{

class ComponentObserver;

  /**
   * \brief Interface for classes implementing delay simulation.
   */
  class Delayer{
  public:
    /**
       * \brief Simulate the delay caused by this Delayer.
       *
       * While this simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(Task& task) = 0;

    virtual const char* getName() const = 0;

    ComponentId getComponentId();

    void addObserver(ComponentObserver *obs);

    void removeObserver(ComponentObserver *obs);
    
    void fireNotification(ComponentInfo *compInf);

  protected:

    Delayer(): componentId(globalComponentId++) {}

    virtual ~Delayer() {}

    typedef std::vector<ComponentObserver *> Observers;
    
    Observers observers;
    
  private:
    //
    static ComponentId globalComponentId;

    //
    ComponentId componentId;
  };

  /**
   * \brief The interface definition to a Virtual-Processing-Component (VPC).
   * 
   * An application using this Framework should call the AbstractComponent::compute(const char *, const char *, sc_event) Funktion.
   */
  class AbstractComponent:
    public sc_module, public Delayer, public ComponentInfo{
  
  public:

    virtual ~AbstractComponent(){}

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
     * \brief Get the process control block.
     */
    ProcessControlBlock* getProcessControlBlock(ProcessId pid)
      throw(NotAllocatedException)
    {
      return this->getPCBPool().allocate(pid);
    }

    /**
     * \brief Create the process control block.
     */
    ProcessControlBlock& createPCB(ProcessId pid){
      return this->getPCBPool().registerPCB(pid);
    }

    /**
     *
     */
    PCBPool& getPCBPool(){
      return *(this->pcbPool);
    }
  protected:

    // points to direct associated controlling instance
    ProcessEventListener* parentControlUnit;
  
  public:
  
    AbstractComponent(sc_module_name name)
      : sc_module(name),
        Delayer() {}
            
    /**
       * \brief Simulate an execution on this "Virtual Component".
       *
       * While this simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(ProcessControlBlock* pcb)=0;

    void compute(Task& task){
      ProcessControlBlock* pcb =
        this->getPCBPool().allocate(task.pid);
      pcb->setBlockEvent(task.blockEvent);
      this->compute(pcb);
    };
    
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
     * \param pcb points to the finished or killed task
     */
    virtual void notifyParentController(ProcessControlBlock* pcb){
      this->parentControlUnit->signalProcessEvent(pcb);
    }

    /**
     * 
     */
    virtual void setPowerMode(const PowerMode& mode){
      if(pcbPools.find(mode) == pcbPools.end()){
        pcbPools[mode] = new PCBPool();
      }
      this->pcbPool =  pcbPools[mode];
    }
    private:
    /**
     *
     */
    PCBPool *pcbPool;
    std::map<PowerMode, PCBPool*> pcbPools;

  };
  
}

#endif //HSCD_VPC_ABSTRACTCOMPONENT_H
