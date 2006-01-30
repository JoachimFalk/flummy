#ifndef HSCD_VPC_ROUNDROBINCONTROLLER_H_
#define HSCD_VPC_ROUNDROBINCONTROLLER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>
#include <vector>

#include <hscd_vpc_Controller.h>
#include <hscd_vpc_Configuration.h>
#include <hscd_vpc_Director.h>

namespace SystemC_VPC{

  /**
   * Implementation of AbstactController which runs FIFO strategy without preempting or killing 
   * running configuraitons.
   * This means that task are served in their arriving order and completed
   * before other conflicting task, which need another configuration, may be
   * completed.
   */
  class RoundRobinController : public Controller {
  
  private:
    // timeslice used for roundrobin
     double TIMESLICE;
     // last time a configuration switch took place
      double lastassign;
      // 
      double remainingSlice;
      
      // indicates if switch should take place
      bool switchConfig;
      
    // queue of tasks ready to be processed
    std::queue<p_struct* > tasksToProcess;
    
    // queue containing order of configuration to be loaded in next "rounds"
    // structure contains additional count of tasks running on one configuration
    std::deque<std::pair<Configuration*, int> > rr_configfifo;
    
    // current scheduled configuration
    std::pair<Configuration*, int>* currConfiguration;
    
  public:
  
    RoundRobinController(const char* name);
          
    virtual ~RoundRobinController();
  
    /**
     * \brief Used to set controller specific values
     * \param key specifies the identy of the property
     * \param value specifies the actual value
     */
    void setProperty(char* key, char* value);
    
    /**
      * \brief Realizes scheduling decision for tasks to be forwarded to configurations
      * This method is used to perform scheduling decision for tasks and within this context
      * their corresponding configurationgs depending on the strategie of the different
      * controller. It is used to initialize and set up all necessary data for a new "round" of
      * scheduling. 
      */
    virtual void addTasksToSchedule(std::deque<p_struct* >& newTasks);
          
     /**
      * \brief Returns next configuration to be loaded
      * Used to indicate if a new configuration should be loaded by the controller
      * component.
      * \return pointer to next configuration to be loaded or NULL if no configuration
      * is selected up to now.
      */
     virtual Configuration* getNextConfiguration();
  
     /**
      * \brief Indicates if controller still can forward tasks
      * \return TRUE if there are still task to be forwarded else FALSE
      */
     virtual bool hasTaskToProcess();
      
     /**
      * \brief Returns next task to be forwarded
      * This method should only be called after calling hasTaskToProcess
      * to ensure that there are still existing task to process.
      * \return pair containing p_struct of task and requested function
      * to be simulated.
      */
     virtual p_struct* getNextTask();
      
     /**
      * \brief Signals if a configuration has to be reactived by controlled component
      * \param config points to configuration which should be reactivated.
      */ 
     bool needToReactivateConfiguration(Configuration* config);

    virtual void signalTaskEvent(p_struct* pcb);
    
    virtual void signalPreemption();
    virtual void signalResume();
    
  private:
    void calculateAssignTime(Configuration* nextConfiguration);
    
    void updateUsedConfigurations(p_struct* pcb);
  };

  /**
   * Predicate class to enable search of std::pair<Configuration*, int>
   * within management structure of controller.
   */
  class SpecialEqual{
  
  private:
    
    Configuration* conf;
    
  public:
    
    SpecialEqual(Configuration* conf): conf(conf){
    }
    
    bool operator()(std::pair<Configuration*, int> p){
      
      return conf == p.first;
      
    }
  };
}
#endif /*HSCD_VPC_ROUNDROBINCONTROLLER_H_*/
