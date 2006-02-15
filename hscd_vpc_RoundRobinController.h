#ifndef HSCD_VPC_ROUNDROBINCONTROLLER_H_
#define HSCD_VPC_ROUNDROBINCONTROLLER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>
#include <vector>

#include "hscd_vpc_Controller.h"
#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_Director.h"

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
    
    /**
     * Helper class to enable internal management of
     * configuration sharing time on managed component.
     */
    class RRElement{
    
    private:
      
      Configuration* conf;
      // number of running tasks on configuration
      int numOfTasks;
      
    public:
      
      RRElement(Configuration* conf): conf(conf), numOfTasks(0){
      }
      
      RRElement(Configuration* conf, int numOfTasks): conf(conf), numOfTasks(numOfTasks){
      }
      
      Configuration* getConfiguration(){
        return conf;
      }
      
      int numberOfTasks(){
        return this->numOfTasks;
      }
      
      void operator++(int){
        this->numOfTasks++;
      }
      
      void operator--(int){
        if(this->numOfTasks > 0){
          this->numOfTasks--;
        }
      }
      
      
      bool operator==(const RRElement& elem){
        
        return this->conf == elem.conf;
        
      }
      
      bool operator==(const int num){
      
        return this->numOfTasks == num;
        
      }
      
    };

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
    std::deque<RRElement> rr_configfifo;
    
    // current scheduled configuration
    RRElement* scheduledConfiguration;
    
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

    /**
     * \see AbstractController
     */
    virtual void signalTaskEvent(p_struct* pcb);
    
    /**
     * \see AbstractController
     */
    virtual void signalPreemption();
    
    /**
     * \see AbstractController
     */
    virtual void signalResume();
    
  private:
  
    /**
     * \brief Helper method to caculate assign time of configuration
     * This method is used to determine the time, when a choosen
     * configuration will be activ, to enable roundrobin to
     * determine when the given timeslice is elapsed.
     */ 
    void calculateAssignTime(Configuration* nextConfiguration);
    
    /**
     * \brief Helper method to keep management structure uptodate
     */
    void updateUsedConfigurations(p_struct* pcb);
  };

}
#endif /*HSCD_VPC_ROUNDROBINCONTROLLER_H_*/
