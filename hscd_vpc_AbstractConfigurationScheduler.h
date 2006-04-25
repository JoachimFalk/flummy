#ifndef HSCD_VPC_ABSTRACTCONFIGURATIONSCHEDULER_H_
#define HSCD_VPC_ABSTRACTCONFIGURATIONSCHEDULER_H_

#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC {
  
  class AbstractController;

  /**
   * \brief Abstract class specify necessary interface of an configuration scheduler
   */
  class AbstractConfigurationScheduler {

    public:

      /**
       * \brief Default constructor
       * \param controller specifies associated controller instance of scheduler
       */

      AbstractConfigurationScheduler() {}

      virtual ~AbstractConfigurationScheduler() {}

      //virtual AbstractController* getController()=0;

      /**
       * \brief Updates management structures for performing schedule decisions
       * This method is used to initialize and set up all necessary data for a new "round" of
       * scheduling. 
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config)=0; 
    
      /**
       * \brief Realizes scheduling decision for tasks to be forwarded to configurations
       * This method is used to perform scheduling decision for tasks and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller.
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */ 
      virtual void performSchedule()=0;
      
      /**
       * \brief Returns next configuration to be loaded
       * Used to indicate if a new configuration should be loaded by the controller
       * component.
       * \return id of next configuration to be loaded or 0 if no configuration
       * is selected up to now.
       */
      virtual unsigned int getNextConfiguration()=0;

      /**
       * \brief Indicates if controller still can forward tasks
       * \return TRUE if there are still task to be forwarded else FALSE
       */
      virtual bool hasTaskToProcess()=0;

      /**
       * \brief Returns next task to be forwarded
       * This method should only be called after calling hasTaskToProcess
       * to ensure that there are still existing task to process.
       * \return pair containing ProcessControlBlock of task and requested function
       * to be simulated.
       */
      virtual ProcessControlBlock* getNextTask()=0;

      /**
       * \brief Callback Mehtode used to inform Controller about task state
       */
      virtual void signalTaskEvent(ProcessControlBlock* pcb)=0;

      virtual sc_time* getWaitInterval()=0;

      /**
       * \brief Used to set Scheduler specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       */
      virtual bool setProperty(char* key, char* value)=0;

      /**
       * \brief Setter to specify if controller should use "kill" by preemption
       */
      virtual void setPreemptionStrategy(bool kill)=0;

      /**
       * \brief Getter to determine which preemption mode is used
       */
      virtual bool preemptByKill()=0;

      /**
       * \brief Signals to controller that managed component has been preempted.
       * Used within controller to adapt scheduling to preemption of managed
       * component.
       */
      virtual void signalPreemption()=0;

      /**
       * \brief Signals to controller that managed component has been resumed.
       * Used within controller to adapt scheduling to resuming of managed
       * component.
       */
      virtual void signalResume()=0;
  };

}
#endif /*HSCD_VPC_ABSTRACTCONFIGURATIONSCHEDULER_H_*/
