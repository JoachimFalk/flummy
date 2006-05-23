#ifndef HSCD_VPC_FCFSCONFSCHEDULER_H_
#define HSCD_VPC_FCFSCONFSCHEDULER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>
#include <vector>

#include "hscd_vpc_ConfigurationScheduler.h"
#include "hscd_vpc_Director.h"


namespace SystemC_VPC{

  /**
   * Implementation of AbstactController which runs FIFO strategy without preempting or killing 
   * running configuraitons.
   * This means that task are served in their arriving order and completed
   * before other conflicting task, which need another configuration, may be
   * completed.
   */
  class FCFSConfScheduler : public ConfigurationScheduler {

    private:

      // queue of waiting tasks to be executed
      std::deque<std::pair<ProcessControlBlock*, unsigned int> > readyTasks;
      // map of running tasks
      std::map<int, ProcessControlBlock* > runningTasks;
      // queue of tasks ready to be processed
      std::queue<ProcessControlBlock* > tasksToProcess;

      // pointer to next configuration to be loaded
      unsigned int nextConfiguration;

    public:

      FCFSConfScheduler(AbstractController* controller);

      virtual ~FCFSConfScheduler();

      /**
       * \brief Updates management structures for performing schedule decisions
       * This method is used to initialize and set up all necessary data for a new "round" of
       * scheduling.
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config);

      /**
       * \brief Realizes scheduling decision for tasks to be forwarded to configurations
       * This method is used to perform scheduling decision for tasks and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller.
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void performSchedule();

      /**
       * \brief Realizes scheduling decision for tasks to be forwarded to configurations
       * This method is used to perform scheduling decision for tasks and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller. It is used to initialize and set up all necessary data for a new "round" of
       * scheduling. 
       */
      //    virtual void addTasksToSchedule(std::deque<ProcessControlBlock* >& newTasks);

      /**
       * \brief Returns next configuration to be loaded
       * Used to indicate if a new configuration should be loaded by the controlled
       * component.
       * \return id of next configuration to be loaded or 0 if no configuration
       * is selected up to now.
       */
      virtual unsigned int getNextConfiguration();

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
       * \brief Implementation of AbstractConfigurationScheduler::signalPreemption
       * \see AbstractConfigurationScheduler
       */
      void signalPreemption(bool kill);

      /**
       * \brief Signals always true as configuration is only switched if all task have finished
       * or have been aborted
       */
      bool preemptByKill(){
        return true;
      } 
  };

}
#endif /*HSCD_VPC_FCFSCONFSCHEDULER_H_*/
