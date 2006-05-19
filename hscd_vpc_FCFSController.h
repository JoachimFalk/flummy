#ifndef HSCD_VPC_FCFSCONTROLLER_H_
#define HSCD_VPC_FCFSCONTROLLER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>
#include <vector>

#include "hscd_vpc_Controller.h"
#include "hscd_vpc_Director.h"

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
    std::deque<ProcessControlBlock* > readyTasks;
    // map of running tasks
    std::map<int, ProcessControlBlock* > runningTasks;
    // queue of tasks ready to be processed
    std::queue<ProcessControlBlock* > tasksToProcess;
    
    // pointer to next configuration to be loaded
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
    virtual void addTasksToSchedule(std::deque<ProcessControlBlock* >& newTasks);
          
    /**
     * \brief Returns next configuration to be loaded
     * Used to indicate if a new configuration should be loaded by the controlled
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
     * \return pair containing ProcessControlBlock of task and requested function
     * to be simulated.
     */
    virtual ProcessControlBlock* getNextTask();
      
    /**
     * \brief Signals if a configuration has to be reactived by controlled component
     * \param config points to configuration which should be reactivated.
     */ 
    bool needToReactivateConfiguration(Configuration* config);
    
    /**
     * \see TaskEventListener
     */
    virtual void signalTaskEvent(ProcessControlBlock* pcb);
    
    /**
     * \brief Signals always true as configuration is only switched if all task have finished
     * or have been aborted
     */
    bool preemptByKill(){
     return true;
    }

    /**
     * \brief Used to indicate preemption to controller
     * This method is used in cases when preemption from higher hierarchy happens.
     * Only preemption with kill influences behaviour of controller as
     * all still waiting task have to be signalled upward to be aborted.
     * \see AbstractController
     */
    void signalPreemption(bool kill);

  };

}
#endif /*HSCD_VPC_FCFSCONTROLLER_H_*/
