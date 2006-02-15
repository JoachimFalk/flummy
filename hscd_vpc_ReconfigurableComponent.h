#ifndef HSCD_VPC_RECONFIGURABLECOMPONENT_H_
#define HSCD_VPC_RECONFIGURABLECOMPONENT_H_

#include <systemc.h>

#include <vector>
#include <map>
#include <deque>

#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_AbstractComponent.h"
#include "hscd_vpc_Configuration.h"

namespace SystemC_VPC{
  
  class ReconfigurableComponent : public AbstractComponent, public sc_module {

    //virtual void compute(p_struct *actualTask);
    
  private:
    
    SC_HAS_PROCESS(ReconfigurableComponent);
    
    // used to create vcd tracefiles for configurations
    sc_trace_file *traceFile;
    map<std::string, sc_signal<trace_value>* > trace_map_by_name;
    
    // pointer to controller of component
    AbstractController* controller;
    // mapping of possible configurations by their name
    std::map<std::string, Configuration* > config_map_by_name;
    // pointer to currently activ Configuration
    Configuration* activConfiguration;
    
    // queue containing new task to be added
    std::deque<p_struct* > newTasks;
    
    // used to notify new tasks to the component
    sc_event notify_schedule_thread;
    // used to indicate preemption
    sc_event notify_preempt;
    // used to indicate resume after preemption
    sc_event notify_resume;
    
    // start time of reconfiguration
    sc_time* storeStartTime;
    // time needed for reconfiguration 
    sc_time* remainingStoreTime;
    
  public:

    /**
     * \brief An implementation of AbstractComponent used together with passive actors and global SMoC v2 Schedulers.
     */
    ReconfigurableComponent(sc_module_name name, AbstractController* controller);
        
    virtual ~ReconfigurableComponent();
  
    /**
     * \brief Executes main task of scheduling and runing task on configurations
     * Within this method all scheduling and reconfiguration of
     * the reconfigurable instance is handled. 
     * Runs as SC_THREAD of ReconfigurableComponent
     */
    void ReconfigurableComponent::schedule_thread();

    /**
     * \brief An implementation of AbstractComponent::compute(const char *, const char *, VPC_Event).
     */
    virtual void compute( const char *name, const char *funcname, VPC_Event *end=NULL);
    
    /**
     * \brief An implementation of AbstractComponent::compute(const char *, VPC_Event).
     */
    virtual void compute( const char *name, VPC_Event *end=NULL);
    
    /**
     * \brief An implementation of AbstractComponent::compute(p_struct*, const char *).
     */
    virtual void compute(p_struct* pcb);
    
    /**
     * \brief Used to create the Tracefiles.
     *
     * To create a vcd-trace-file in SystemC all the signals to 
     * trace have to be in a "global" scope. The signals have to 
     * be created in elaboration phase (before first sc_start).
     */
    virtual void informAboutMapping(std::string module);
    
    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void processAndForwardParameter(char *sType,char *sValue);
    
    /**
     * \brief Preempts execution of component
     * Used to preempt the current execution of a component.
     * \sa AbstractComponent
     */
    virtual void preempt(bool kill);
    
    /**
     * \brief Resumes preempted execution
     * Used to resume execution of preempted component.
     */
    virtual void resume();
    
    /**
     * \see IPreemptable::timeToPreempt
     */
    virtual sc_time timeToPreempt();
    
    /**
     * \see IPreemptable::timeToResume
     */
    virtual sc_time timeToResume();
    
    /**
     * \brief Adds new Configuration to the reconfigurable component.
     * \param config the Configuration to be added
     */
    void addConfiguration(const char* name, Configuration* config);
      
    /**
     * \brief Enables access to known Configurations of component
     * Used to map vector of known Configurations of current instance.
     */
    std::map<std::string, Configuration* >& getConfigurations();
    
    /**
      * \brief Enables access to a single Component by its identifying name
      * \param name specifies the component to retrieve
      * \return requested component or NULL if identifier of component
      * is not known.
     */
    Configuration* getConfiguration(const char* name);

    /**
     * \brief Gets currently loaded Configuration
     */
    Configuration* getActivConfiguration();
      
    /**
     * \brief Sets currently loaded Configuration
     * \param name specifies the Configuration to be set as activ on
     */
    /**
     * \brief Sets the currently loaded Configuration
     * \param identifying name of Configuration to be set loaded
     */
    void setActivConfiguration(const char* name);
    
    /**
     * \brief Gets associated controller of component
     * \return controller of component
     */  
    AbstractController* getController();
      
    /**
     * \brief Sets controller for current instance
     * \param controllertype specifies the type of requested controller to be
     * associated to the component.
     */
    //void setController(const char* controllertype);
  
    /**
     * \brief Sets controller for current instance
     * \param controller references the requested controller to be
     * associated to the component.
     */
    void setController(AbstractController* controller);
    
    /**
     * \brief Used to enable notification initialized from Controller side
     * This method is called if controller gets notified of finished or killed
     * task, which need new scheduling decisions.
     */
    inline void wakeUp(){
      this->notify_schedule_thread.notify();
    }
    
  private:
  
    /**
     * \brief Loads a given configuration
     * Loads a given configuration by simulating loadtime.
     * \note Loading is only performed if new configuration is unequal to activ configuration
     * \param newConfig refers to the configuration to be loaded
     */ 
    bool loadConfiguration(Configuration* config);
    
    /**
     * \brief Stores activ configuration
     * Stores currently activ configuration regarding to the passed
     * parameter kill, which indicates if configuration should be stored.
     * After successful storing activConfiguration is set to NULL.
     * \param kill specifies if currently loaded configuration has to be stored
     */
    bool storeActivConfiguration(bool kill);
    
    /**
     * \brief Helper method to determine interruption
     */
    bool reconfigurationInterrupted(sc_time timeStamp, sc_time interval);
    
    void ReconfigurableComponent::traceConfigurationState(Configuration* config, trace_value value);
  };

}

#endif /*HSCD_VPC_RECONFIGURABLECOMPONENT_H_*/
