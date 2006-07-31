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
   * This means that process are served in their arriving order and completed
   * before other conflicting process, which need another configuration, may be
   * completed.
   */
  class FCFSController : public Controller {
  
  private:
   
    // queue of waiting processes to be executed
    std::deque<ProcessControlBlock* > readyTasks;
    // map of running processes
    std::map<int, ProcessControlBlock* > runningTasks;
    // queue of processes ready to be processed
    std::queue<ProcessControlBlock* > tasksToProcess;
    
    // pointer to next configuration to be loaded
    Configuration* nextConfiguration;
    
  public:
  
    FCFSController(const char* name);
          
    virtual ~FCFSController();
    
    /**
      * \brief Realizes scheduling decision for processes to be forwarded to configurations
      * This method is used to perform scheduling decision for processes and within this context
      * their corresponding configurationgs depending on the strategie of the different
      * controller. It is used to initialize and set up all necessary data for a new "round" of
      * scheduling. 
      */
    virtual void addProcessToSchedule(std::deque<ProcessControlBlock* >& newTasks);
          
    /**
     * \brief Returns next configuration to be loaded
     * Used to indicate if a new configuration should be loaded by the controlled
     * component.
     * \return pointer to next configuration to be loaded or NULL if no configuration
     * is selected up to now.
     */
    virtual Configuration* getNextConfiguration();
      
    /**
     * \brief Indicates if controller still can forward processes
     * \return TRUE if there are still process to be forwarded else FALSE
     */
    virtual bool hasProcessToDispatch();
      
    /**
     * \brief Returns next process to be forwarded
     * This method should only be called after calling hasProcessToDispatch
     * to ensure that there are still existing process to process.
     * \return pair containing ProcessControlBlock of process and requested function
     * to be simulated.
     */
    virtual ProcessControlBlock* getNextProcess();
      
    /**
     * \brief Signals if a configuration has to be reactived by controlled component
     * \param config points to configuration which should be reactivated.
     */ 
    bool needToReactivateConfiguration(Configuration* config);
    
    /**
     * \see ProcessEventListener
     */
    virtual void signalProcessEvent(ProcessControlBlock* pcb);
    
    /**
     * \brief Signals always true as configuration is only switched if all process have finished
     * or have been aborted
     */
    bool deallocateByKill(){
     return true;
    }

    /**
     * \brief Used to indicate deallocation to controller
     * This method is used in cases when preemption from higher hierarchy happens.
     * Only deallocation with kill influences behaviour of controller as
     * all still waiting process have to be signalled upward to be aborted.
     * \see AbstractController
     */
    void signalDeallocation(bool kill);

  };

}
#endif /*HSCD_VPC_FCFSCONTROLLER_H_*/
