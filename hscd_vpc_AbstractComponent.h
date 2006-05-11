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

#include <systemc.h>

#include <string.h>

#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_IPreemptable.h"
#include "hscd_vpc_TaskEventListener.h"

namespace SystemC_VPC{
  
  /**
   * \brief The interface definition to a Virtual-Processing-Component (VPC).
   * 
   * An application using this Framework should call the AbstractComponent::compute(const char *, const char *, sc_event) Funktion.
   */
  class AbstractComponent: public sc_module, public IPreemptable{
  
  protected:

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     * \deprecated since dynamic allocation has been introduced
     */
    virtual void _compute( const char *name, const char *funcname, VPC_Event* end=NULL)
      __attribute__ ((deprecated)) =0;

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     * \deprecated since dynamic allocation has been introduced
     */
    virtual void _compute( const char *name, VPC_Event *end=NULL)
      __attribute__ ((deprecated)) =0;
   
  public:

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     * \deprecated since dynamic allocation has been introduced
     */
    void __attribute__ ((deprecated)) compute( const char *name, const char *funcname, VPC_Event* end=NULL){
      _compute( name, funcname, end);
    }

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     * \deprecated since dynamic allocation has been introduced
     */
    void __attribute__ ((deprecated)) compute( const char *name, VPC_Event *end=NULL){
      _compute( name, end);
    }

    virtual ~AbstractComponent(){};

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

  protected:
     
    // used to reflect state of component
    bool activ;
  
    // used to reflect if components execution has been killed
    bool killed;
  
    // points to direct associated controlling instance
    TaskEventListener* parentControlUnit;
  
  public:
  
    AbstractComponent(sc_module_name name) : sc_module(name){
      
      this->killed = false;
      this->activ = true;
      
    }
    
    /**
     * \brief Getter to determine if component is set to activ
     */
    inline bool isActiv(){
      
      return this->activ;
      
    }
    
    /**
     * \brief Setter to set value of component to activ or not
     */
    inline void setActiv(bool newVal){
      
      this->activ = newVal;
      
    }
    
    /**
     * \brief Getter to determine if kill has been called on component
     */
    inline bool hasBeenKilled(){
      
      return this->killed;
      
    }
    
    /**
     * \brief Default implementation of IPreemptable::timeToPreempt
     * Default implementation just returns SC_ZERO_TIME
     * \return sc_time with value SC_ZERO_TIME
     */
    virtual sc_time timeToPreempt(){
      return sc_time(SC_ZERO_TIME);
    }
    
    /**
     * \brief Default implementation of IPreemptable::timeToResume
     * Default implementation just returns SC_ZERO_TIME
     * \return sc_time with value SC_ZERO_TIME
     */  
    virtual sc_time timeToResume(){
      return sc_time(SC_ZERO_TIME);
    }
    
    /**
       * \brief Simulate an execution on this "Virtual Component".
       *
       * While this simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(ProcessControlBlock* pcb)=0;
    
    /**
     * \brief Sets next controlling instance of component
     * Used for callback mechanism to store pointer to "direct" controlling instance
     * for later infroming about finished or killed tasks.
     * \param controller points to controlling instance which is
     * responsible for component.
     */
    virtual void setParentController(TaskEventListener* controller){
    
      this->parentControlUnit = controller;
    
    }
    
    /**
     * \brief Notifies parent controlling instance about task event
     * This mehtod is used to inform "direct" controlling instance about
     * finished or killed tasks.
     * \param pcb points to the finished or killed task
     */
    virtual void notifyParentController(ProcessControlBlock* pcb){
      this->parentControlUnit->signalTaskEvent(pcb);
    }

  };
  
}

#endif //HSCD_VPC_ABSTRACTCOMPONENT_H
