#ifndef HSCD_VPC_PRIORITYCONTROLLER_H_
#define HSCD_VPC_PRIORITYCONTROLLER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>


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
  class PriorityController : public Controller {
  
  private:
  
    int order_count;
    
    /**
     * Helper class to store configurations within priority list of controller.
     * Contains pointer to actual configuration and fifo entry for secundary strategy.
     */
    class PriorityListElement{
      
      private:
        Configuration* config;
        int fifo_count;
      
      public:
        PriorityListElement(Configuration* config, int degree) : config(config), fifo_count(degree){}
        
        Configuration* getConfiguration(){
          return this->config;
        }
        
        bool operator < (const PriorityListElement& pe){
          if(this->config->getPriority() > pe.config->getPriority()){
            return true;
          }else if(this->config->getPriority() == pe.config->getPriority()){
            return this->fifo_count > pe.fifo_count;
          }
          
          return false;  
        }
        
        bool operator == (const Configuration* config){
          
          return this->config == config;
          
        }
    };
      
    // queue of waiting tasks to be executed
    std::queue<p_struct* > readyTasks;
    // map of running tasks
    std::map<int, p_struct* > runningTasks;
    // queue of tasks ready to be processed
    std::queue<p_struct* > tasksToProcess;
    
    // queue containing order of configuration to be loaded in next "rounds"
    std::list<PriorityListElement> nextConfigurations;
    
  public:
  
    PriorityController(const char* name);
          
    virtual ~PriorityController();
      
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
     * \brief Used to signal finished tasks to the controller
     * \see TaskEventListener::signalTaskEvent
     */
    virtual void signalTaskEvent(p_struct* pcb);
  
  };

}
#endif /*HSCD_VPC_PRIORITYCONTROLLER_H_*/
