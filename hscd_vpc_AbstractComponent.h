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

//#include <cosupport/systemc_support.hpp>
#include <systemc_support.hpp>

#include <hscd_vpc_datatypes.h>

namespace SystemC_VPC{

  /**
   * \brief The interface definition to a Virtual-Processing-Component (VPC).
   * 
   * An application using this Framework should call the AbstractComponent::compute(const char *, const char *, sc_event) Funktion.
   */
  class AbstractComponent{
  public:

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute( const char *name, const char *funcname, CoSupport::SystemC::Event *end=NULL)=0;

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute( const char *name, CoSupport::SystemC::Event *end=NULL)=0;
    //    virtual void compute(int iprocess)=0;
   
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


    /**************************/
    /*  EXTENSION SECTION     */
    /**************************/

  protected:
     
    // used to reflect state of component
    bool activ;

	// name of component
	char componentName [VPC_MAX_STRING_LENGTH];
	
  public:

	/**
	 * \brief Getter for name of component
	 */
	char* getName(){
		
		return this->componentName;
		
	}

	/**
	 * \brief Getter to determine if component is set to activ
	 */
	bool isActiv(){
		
		return this->activ;
		
	}
	
	/**
	 * \brief Setter to set value of component to activ or not
	 */
	void setActiv(bool newVal){
		
		this->activ = newVal;
		
	}
	
    /**
     * \brief Preempts execution of component
     * Used to preempt the current execution of a component.
     * Actual executed tasks are "stored" for late execution
     */
    virtual void preempt()=0;

    /**
     * \brief Resumes preempted execution
     * Used to resume execution of preempted component.
     */
    virtual void resume()=0;

    /**
     * \brief Determines minimum time till next idle state of component
     * Used to determine how long it will take till component finishes
     * currently running tasks.
     * \return sc_time specifying the time till idle
     */
    virtual sc_time* minTimeToIdle()=0;

	/**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
	virtual void compute( p_struct* pcb, const char *funcname)=0;
		
    /**************************/
    /*  END OF EXTENSION      */
    /**************************/

  };
  
}

#endif //HSCD_VPC_ABSTRACTCOMPONENT_H
