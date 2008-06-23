#ifndef HSCD_VPC_CONFIGURATION_H_
#define HSCD_VPC_CONFIGURATION_H_

#include <systemc.h>

#include <map>
#include <list>
#include <string>

#include "hscd_vpc_AbstractComponent.h"
#include "hscd_vpc_IDeallocatable.h"

namespace SystemC_VPC{
  
  /**
   * \brief Interface definition of Configuration
   * A Configuration represents a set of components which can
   * be dynamically loaded by a reconfigurable Component represented
   * by an instance of ReconfigurableComponent.
   */
  class Configuration : public IDeallocatable{

  private:
    
    // identifies instance of configuration
    char configName[VPC_MAX_STRING_LENGTH];

    // map associating AbstractComponents with their identifying names
    std::map<std::string, AbstractComponent* > component_map_by_name;

    // signals if configuration is activ or not
    bool activ;
    
    // signals if configuration has been killed
    bool killed;
    
    // signals if configuration has been stored successful
    bool stored;
    
    // represents time needed to store configuration
    sc_time storeTime;
    // represents time needed to load configuration
    sc_time loadTime;
    
  public:
    
    /**
     * \brief Creates instance of Configuration
     */
    Configuration(const char* name);
    
    Configuration(const char* name, const char* loadTime, const char* storeTime);
    
    virtual ~Configuration();
    
    /**
     * \brief Getter to access name of Configuration
     */
    char* getName();
    
    /**
     * \brief Getter to access activ flag of configuration
     */
    bool isActiv();
    
    /**
     * \brief Getter to check if configuration has been stored
     */
    bool isStored();
    
    /**
     * \brief Setter to set if configuration has been stored
     */
    void setStored(bool stored);
    
    /**
     * \brief Adds a given component to current configuration
     * Adds a given component to the configuration and deactivates it,
     * if configuration is inactiv. As configurations are inactiv at
     * initiazation time, normally all added components are also deactivated!
     * \param name specifies the identifying name of the component
     * \param comp points to the component to be added
     * \return TRUE if component could be added else FALSE
     */
    bool addComponent(const char* name, AbstractComponent* comp);
    
    /**
     * \brief Enables access to components of configuration
     * Used to enable acces to components of given configuration.
     * \param name specifies the requested component
     * \return requested component or NULL if component is no
     *         member of the configuration
     */
    AbstractComponent* getComponent(const char* name);
      
    /**
     * \brief Preempts execution of components within current configuration
     * Used to deallocate the current execution of components.
     * Actual executed tasks are "stored" for later execution or discarded
     * depending on the parameter flag.
     * \param killTasks indicates if "hard" deallocation should be initiated
     * and currently registered task are killed without restoring
     * \return pointer to sc_time specifying time need for deallocation
     * \see IDeallocatable::deallocate
     */
    void deallocate(bool killTasks);
    
    /**
     * \brief Resumes deallocated execution
     * Used to allocate execution of deallocated configuration which
     * mean the equalent of resuming all its contained components.
     * \see IDeallocatable::allocate
     */
    void allocate();
    
    /**
     * \brief Gets time needed to deallocate all components of configuration
     * Used to determine time needed to store all execution information, if current state
     * of configuration should be recoverable after deallocation.
     * \return time needed if configuration actually is deallocated
     * \see IDeallocatable::timeToDeallocate
     */
    sc_time timeToDeallocate();
    
    /**
     * \brief Gets time needed to restore all components of configuration after deallocation
     * \return time needed for restoring state before deallocation
     * \see IDeallocatable::timeToAllocate
     */
    sc_time timeToAllocate();
    
    /**
     * \brief Determines minimal time of configuration until all components will be idle
     * Used to determine how long it will take till all components within the configuration 
     * finish their currently running executions.
     * \return sc_time specifying the time till idle
     */
    sc_time* minTimeToIdle();

    /**
     * \brief Sets store time for a configuration
     * \param time specifies the corresponding store time
     */
    void setStoreTime(const char* time);
    
    /**
     * \brief Getter to access store time of configuration
     * Grants access to store time of configuration neglecting additional
     * time needed for storing encapsulated components.
     * \return sc_time specifying time to store configuration
     */
    const sc_time& getStoreTime();
    
    /**
     * \brief Sets store time for a configuration
     * \param time specifies the corresponding store time
     */
    void setLoadTime(const char* time);
    
    /**
     * \brief Getter to acces load time of configuration
     * Grants access to load time of configuration neglecting additional
     * time needed for loading encapsulated components.
     * \return sc_time specifying time to load configuration
     */
    const sc_time& getLoadTime();
    
  };
 
}

#endif /*HSCD_VPC_CONFIGURATION_H_*/ 