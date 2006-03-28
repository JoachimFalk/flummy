#ifndef HSCD_VPC_ABSTRACTCONTROLLER_H_
#define HSCD_VPC_ABSTRACTCONTROLLER_H_

#include <string>
#include <map>
#include <queue>
#include <deque>
#include <vector>

#include "hscd_vpc_AbstractDirector.h"
#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_TaskEventListener.h"

#include <hscd_vpc_InvalidArgumentException.h>

namespace SystemC_VPC{
  
  class ReconfigurableComponent;
  
  /**
   * \brief Specifies the interface provided by a controller.
   * This abstract class is used to declare a common interface for all controller
   * used within the VPC framework.
   */
  class AbstractController : public AbstractDirector{
        
  public:

    virtual ~AbstractController(){}
    
    /**
     * \brief Getter for controller name
     */
    virtual char* getName()=0;
      
    /**
     * \brief Sets the currently controlled reconfigurable Component of instance
     */
    virtual void setManagedComponent(ReconfigurableComponent* managedComponent)=0;
    
    /**
     * \brief Gets the currently conrtolled reconfigurable Component of instance
     */
    virtual ReconfigurableComponent* getManagedComponent()=0;
    
    /**
     * \brief Returns time to wait until next notification of controller is needed
     * Returns time interval indicating when controlled component should invoke controller
     * next time.
     * \return time interval to wait or NULL if no time interval required
     */
    virtual sc_time* getWaitInterval()=0;
    
    /**
     * \brief Used to set controller specific values
     * \param key specifies the identy of the property
     * \param value specifies the actual value
     */
    virtual void setProperty(char* key, char* value)=0;
    
    /**
     * \brief Realizes scheduling decision for tasks to be forwarded to configurations
     * This method is used to perform scheduling decision for tasks and within this context
     * their corresponding configurationgs depending on the strategie of the different
     * controller. It is used to initialize and set up all necessary data for a new "round" of
     * scheduling. 
     */
    virtual void addTasksToSchedule(std::deque<p_struct* >& newTasks)=0;
     
    /**
     * \brief Returns next configuration to be loaded
     * Used to indicate if a new configuration should be loaded by the controller
     * component.
     * \return pointer to next configuration to be loaded or NULL if no configuration
     * is selected up to now.
     */
    virtual Configuration* getNextConfiguration()=0;
      
    /**
     * \brief Returns mapped component for a given task
     * \param task specifies the task to get component for
     * \return pointer to AbstractComponent refering to mapped component
     */
    virtual AbstractComponent* getMappedComponent(p_struct* task)=0;
      
    /**
     * \brief Indicates if controller still can forward tasks
     * \return TRUE if there are still task to be forwarded else FALSE
     */
    virtual bool hasTaskToProcess()=0;

    /**
     * \brief Returns next task to be forwarded
     * This method should only be called after calling hasTaskToProcess
     * to ensure that there are still existing task to process.
     * \return pair containing p_struct of task and requested function
     * to be simulated.
     */
    virtual p_struct* getNextTask()=0;
    
    /**
     * \brief Setter to specify if controller should use "kill" by preemption
     */
    virtual void setPreemptionStrategy(bool kill)=0;
    
    /**
     * \brief Getter to determine which preemption mode is used
     */
    virtual bool preemptByKill()=0;
        
    /**
     * \brief Callback Mehtode used to inform Controller about task state
     */
    virtual void signalTaskEvent(p_struct* pcb)=0;
    
    /**
     * \brief Signals to controller that managed component has been preempted.
     * Used within controller to adapt scheduling to preemption of managed
     * component.
     */
    virtual void signalPreemption()=0;
    
    /**
     * \brief Signals to controller that managed component has been resumed.
     * Used within controller to adapt scheduling to resuming of managed
     * component.
     */
    virtual void signalResume()=0;
    
  };

}

#endif /*HSCD_VPC_ABSTRACTCONTROLLER_H_*/
