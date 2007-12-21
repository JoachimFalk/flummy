#ifndef HSCD_VPC_ABSTRACTCONTROLLER_H_
#define HSCD_VPC_ABSTRACTCONTROLLER_H_

#include <string>
#include <map>
#include <queue>
#include <deque>
#include <vector>

#include "hscd_vpc_AbstractDirector.h"

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_AbstractConfigurationMapper.h"
#include "hscd_vpc_AbstractAllocator.h"

#include "hscd_vpc_ProcessEventListener.h"

#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_datatypes.h"

#include <hscd_vpc_InvalidArgumentException.h>

namespace SystemC_VPC{
  
  class ReconfigurableComponent;
  
  /**
   * \brief helper struct to remember current decision for pcb
   */
  struct Decision {
    ProcessControlBlock* pcb;
    std::string comp;
    unsigned int conf;
  };
    
  /**
   * \brief Specifies the interface provided by a controller.
   * This abstract class is used to declare a common interface for all controller
   * used within the VPC framework.
   */
  class AbstractController : public AbstractDirector{
        
  public:

    virtual ~AbstractController() {}
    
    virtual void initController(char*)=0;
    
    /**
     * \brief Getter for controller name
     */
    virtual char* getName()=0;

    virtual AbstractBinder* getBinder()=0;
    virtual AbstractConfigurationMapper* getConfigurationMapper()=0;
    virtual AbstractAllocator* getAllocator()=0;
    
    /**
     * \brief Sets the currently controlled reconfigurable Component of instance
     */
    virtual void setManagedComponent(ReconfigurableComponent* managedComponent)=0;
    
    /**
     * \brief Gets the currently controlled reconfigurable Component of instance
     */
    virtual ReconfigurableComponent* getManagedComponent()=0;
    
    /**
     * \brief Returns time to wait until next notification of controller is needed
     * Returns time interval indicating when controlled component should invoke controller
     * next time.
     * \return time interval to wait or NULL if no time interval required
     */
    virtual sc_time* getWaitInterval(ReconfigurableComponent* rc)=0;
    
    /**
     * \brief Used to set controller specific values
     * \param key specifies the identy of the property
     * \param value specifies the actual value
     */
    virtual void setProperty(char* key, char* value)=0;
    
    /**
     * \brief Realizes scheduling decision for processes to be forwarded to configurations
     * This method is used to perform scheduling decision for processes and within this context
     * their corresponding configurationgs depending on the strategie of the different
     * controller. It is used to initialize and set up all necessary data for a new "round" of
     * scheduling. 
     * \param newTasks refers to a queue of pcb to be scheduled
     */
    virtual void addProcessToSchedule(std::deque<ProcessControlBlock* >& newTasks, ReconfigurableComponent* rc)=0;

    virtual void performSchedule(ReconfigurableComponent* rc) =0;

    /**
     * \brief Returns next configuration to be loaded
     * Used to indicate if a new configuration should be loaded by the controller
     * component.
     * \return id of next configuration to be loaded or 0 if no configuration
     * is selected up to now.
     */
    virtual unsigned int getNextConfiguration(ReconfigurableComponent* rc)=0;
      
    /**
     * \brief Returns mapped component for a given process
     * \param process specifies the process to get component for
     * \return pointer to AbstractComponent refering to mapped component
     */
    virtual AbstractComponent* getMappedComponent(ProcessControlBlock* process, ReconfigurableComponent* rc)=0;
      
    /**
     * \brief Indicates if controller still can forward processes
     * \return TRUE if there are still process to be forwarded else FALSE
     */
    virtual bool hasProcessToDispatch(ReconfigurableComponent* rc)=0;

    /**
     * \brief Returns next process to be forwarded
     * This method should only be called after calling hasProcessToDispatch
     * to ensure that there are still existing process to process.
     * \return pair containing ProcessControlBlock of process and requested function
     * to be simulated.
     */
    virtual ProcessControlBlock* getNextProcess(ReconfigurableComponent* rc)=0;
    
    /**
     * \brief Getter to determine which deallocation mode is used
     */
    virtual bool deallocateByKill()=0;
        
    /**
     * \brief Callback Mehtode used to inform Controller about process state
     */
    virtual void signalProcessEvent(ProcessControlBlock* pcb, std::string compID)=0;
    
    /**
     * \brief Signals to controller that managed component has been deallocated.
     * Used within controller to adapt scheduling to deallocation of managed
     * component.
     * \param kill indicates if deallocation used kill as parameter
     */
    virtual void signalDeallocation(bool kill, ReconfigurableComponent* rc)=0;
    
    /**
     * \brief Signals to controller that managed component has been reallocated.
     * Used within controller to adapt scheduling to resuming of managed
     * component.
     */
    virtual void signalAllocation(ReconfigurableComponent* rc)=0;
    
    /**
     * \brief 
     */
    virtual Decision getDecision(int pid, ReconfigurableComponent* rc)=0;

    virtual sc_time getSchedulingOverhead()=0;    
  };

}

#endif /*HSCD_VPC_ABSTRACTCONTROLLER_H_*/
