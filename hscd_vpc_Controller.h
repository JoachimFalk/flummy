#ifndef HSCD_VPC_CONTROLLER_H_
#define HSCD_VPC_CONTROLLER_H_

#include <string>
#include <map>
#include <queue>
#include <deque>
#include <vector>

#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC{
  
  /**
   * \brief Implementation of base methods specified by AbstractController
   * This class realizes basic function declared by AbstractController, which
   * are used by all subclasses. If different behaviour is required, the specific
   * methods can be overloaded by the subclasses..
   * \see AbstractController
   */
  class Controller : public AbstractController{
  
  private: 
    char controllerName [VPC_MAX_STRING_LENGTH];
  
    // controlled component of instance
    ReconfigurableComponent* managedComponent;
    
    // maps tasks to their corresponding names of component
    std::map<std::string, std::string > mapping_map_component_ids;
    
    /*
     * map of maps for reconfigration times
     * first key specifies current configuration
     * second key specifies next configuration
     * value specifies the time needed to switch btw first and second
     */
    //std::map<std::string, sc_time> loadTime_map;
    //std::map<std::string, sc_time> storeTime_map;
    
    // true if controller uses kill to preempt configurations
    bool kill;
  
  protected:
    
    // time indicating next request wish
    sc_time* waitInterval;
    
    // maps tasks to their corresponding names of configuration
    std::map<std::string, std::string > mapping_map_configs;
        
  public:
    
    Controller(const char* name);
    
    virtual ~Controller(){}

    /**
     * \brief Getter for controller name
     */
    char* getName();
      
    /**
     * \brief Sets the currently controlled reconfigurable Component of instance
     */
    void setManagedComponent(ReconfigurableComponent* managedComponent);
    
    /**
     * \brief Gets the currently conrtolled reconfigurable Component of instance
     */
    ReconfigurableComponent* getManagedComponent();
    
    /**
     * \brief Returns time to wait until next notification of controller is needed
     * Returns time interval indicating when controlled component should invoke controller
     * next time.
     * \return time interval to wait or NULL if no time interval required
     */
    sc_time* getWaitInterval();
    
    /**
     * \brief Register component to Director
     * Used to register a component to the Director for
     * later computation of task on it. The components name
     * is used as identifier for it.
     * \param comp points to component instance to be registered
     */
    virtual void registerComponent(AbstractComponent* comp);
      
    /**
     * \brief Registers mapping between task and component to Director
     * \param taskName specifies name of task
     * \param compName specifies name of component
     */
    virtual void registerMapping(const char* taskName, const char* compName);
              
    /**
     * \brief Returns mapped component for a given task
     * \param task specifies the task to get component for
     * \return pointer to AbstractComponent refering to mapped component
     */
    virtual AbstractComponent* getMappedComponent(p_struct* task);
    
    /**
     * \brief Used to set controller specific values
     * \param key specifies the identy of the property
     * \param value specifies the actual value
     */
    virtual void setProperty(char* key, char* value);
    
    /**
     * \brief Setter to specify if controller should use "kill" by preemption
     */
    void setPreemptionStrategy(bool kill);
    
    /**
     * \brief Getter to determine which preemption mode is used
     */
    bool preemptByKill();  
  
    /**
     * \brief Signals to controller that managed component has been preempted.
     * Used within controller to adapt scheduling to preemption of managed
     * component.
     * \note Does nothing intended for controllers not interested in preemption
     */
    virtual void signalPreemption();
    
    /**
     * \brief Signals to controller that managed component has been resumed.
     * Used within controller to adapt scheduling to resuming of managed
     * component.
     * \note Does nothing intended for controllers not interested in resume
     */
    virtual void signalResume();
    
  protected:
    
    /**
     * \brief Helper method to enable easy access to mapped coniguration
     * Intended for internal use within controller realization to have
     * single point of implementation of this rudimentary functionality
     */
    Configuration* getMappedConfiguration(const char* name);    
  };

}

#endif /*HSCD_VPC_CONTROLLER_H_*/
