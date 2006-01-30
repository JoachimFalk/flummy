#ifndef HSCD_VPC_CONFIGURATION_H_
#define HSCD_VPC_CONFIGURATION_H_

#include <systemc.h>

#include <map>
#include <list>
#include <string>

#include <hscd_vpc_AbstractComponent.h>
#include <hscd_vpc_IPreemptable.h>

namespace SystemC_VPC{
  
  /**
   * \brief Interface definition of Configuration
   * A Configuration represents a set of components which can
   * be dynamically loaded by a reconfigurable Component represented
   * by an instance of ReconfigurableComponent.
   */
  class Configuration : public IPreemptable{

  private:
    
    // identifies instance of configuration
    char configName[VPC_MAX_STRING_LENGTH];

    // map associating AbstractComponents with their identifying names
    std::map<std::string, AbstractComponent* > component_map_by_name;

    // list of priorities
    std::list<int> priorities;
    
    // list of deadlines
    std::list<double> deadlines;
    
    // signals if configuration is activ or not
    bool activ;
    
    // signals if configuration has been killed
    bool killed;
    
    // signals if configuration has been stored successful
    bool stored;
    
    sc_time storeTime;
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
     * Used to preempt the current execution of components.
     * Actual executed tasks are "stored" for later execution or discarded
     * depending on the parameter flag.
     * \param kill indicates if "hard" preemption should be initiated
        * and currently registered task are killed without restoring
        * \return pointer to sc_time specifying time need for preemption
        * \see IPreemptable::preempt
     */
    void preempt(bool killTasks);
    
    /**
     * \brief Resumes preempted execution
     * Used to resume execution of preempted configuration which
     * mean the equalent of resuming all its contained components.
     * \see IPreemptable::resume
     */
    void resume();
    
    /**
     * \brief Gets time needed to preempt all components of configuration
     * Used to determine time needed to store all execution information, if current state
     * of configuration should be recoverable after preemption.
     * \return time needed if configuration actually is preempted
     * \see IPreemptable::timeToPreempt
     */
    sc_time* timeToPreempt();
    
    /**
     * \brief Gets time needed to restore all components of configuration after preemption
     * \return time needed for restoring state before preemption
     * \see IPreemptable::timeToResume
     */
    sc_time* timeToResume();
    
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
    
    /**
     * \brief Adds a priority to a configuration
     * Used to add priority of a running task on a configuration 
     * to the priority of the configuration to enable priority schedule
     * \param p specifies the priority to be added
     */
    void addPriority(int p);
    
    /**
     * \brief Removes a priority from the configuration
     * Used to remove priority of a finished task from the configuration,
     * the first occurence of the priority will be removed.
     * \param p specifies the priority value to be removed
     */
    void removePriority(int p);
    
    /**
     * \brief Access to current priority of the configuration
     * Used to access the current priority of a configuration determined
     * by the task running on it. If there are no running tasks
     * a default value is returned.
     * \return current priority of configuration or -1 if no task is running
     * on the configuration
     */
    const int getPriority();
    
    /**
     * \brief Adds a deadline to a configuration
     * Used to add deadline of a running task on a configuration 
     * to the deadline of the configuration to enable deadline schedule
     * \param p specifies the deadline to be added
     */
    void addDeadline(double d);
    
    /**
     * \brief Removes a deadline from the configuration
     * Used to remove deadline of a finished task from the configuration,
     * the first occurence of the deadline will be removed.
     * \param p specifies the deadline value to be removed
     */
    void removeDeadline(double d);
    
    /**
     * \brief Access to current deadline of the configuration
     * Used to access the current deadline of a configuration determined
     * by the task running on it. If there are no running tasks
     * a default value is returned.
     * \return current deadline of configuration or -1 if no task is running
     * on the configuration
     */
    const double getDeadline();
  };
 
}

#endif /*HSCD_VPC_CONFIGURATION_H_*/ 
