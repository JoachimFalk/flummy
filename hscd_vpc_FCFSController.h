#ifndef HSCD_VPC_FCFSCONTROLLER_H_
#define HSCD_VPC_FCFSCONTROLLER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>
#include <vector>

#include <hscd_vpc_Controller.h>
#include <hscd_vpc_Director.h>

namespace SystemC_VPC{

  /**
   * Implementation of AbstactController which runs FIFO strategy without preempting or killing 
   * running configuraitons.
   * This means that task are served in their arriving order and completed
   * before other conflicting task, which need another configuration, may be
   * completed.
   */
  class FCFSController : public Controller {
  
  private:
   
    // queue of waiting tasks to be executed
    std::queue<p_struct* > readyTasks;
    // map of running tasks
    std::map<int, p_struct* > runningTasks;
    // queue of tasks ready to be processed
    std::queue<p_struct* > tasksToProcess;
    
    // queue containing order of configuration to be loaded in next "rounds"
    //std::queue<Configuration* > nextConfigurations;
    Configuration* nextConfiguration;
    
  public:
  
    FCFSController(const char* name);
          
    virtual ~FCFSController();
    
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
    
    /**
     * \see TaskEventListener
     */
    virtual void signalTaskEvent(p_struct* pcb);
    
  };

}
#endif /*HSCD_VPC_FCFSCONTROLLER_H_*/
