#ifndef HSCD_VPC_ROUNDROBINCONFSCHEDULER_H_
#define HSCD_VPC_ROUNDROBINCONFSCHEDULER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>
#include <vector>

#include "hscd_vpc_ConfigurationScheduler.h"
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
  class RoundRobinConfScheduler : public ConfigurationScheduler {

    private:

      /**
       * Helper class to enable internal management of
       * configuration sharing time on managed component.
       */
      class RRElement{

        private:

          //Configuration* conf;
          unsigned int config;
          // number of running tasks on configuration
          int numOfTasks;

        public:

          RRElement(unsigned int conf): config(conf), numOfTasks(0){
          }

          RRElement(unsigned int conf, int numOfTasks): config(conf), numOfTasks(numOfTasks){
          }

          unsigned int getConfiguration(){
            return config;
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

            return this->config == elem.config;

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
      std::queue<ProcessControlBlock* > tasksToProcess;

      // queue containing order of configuration to be loaded in next "rounds"
      // structure contains additional count of tasks running on one configuration
      std::deque<RRElement> rr_configfifo;

      // current scheduled configuration
      RRElement* scheduledConfiguration;

    public:

      RoundRobinConfScheduler(AbstractController* controller);

      virtual ~RoundRobinConfScheduler();

      /**
       * \brief Used to set controller specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if value has been used else false
       * \sa ConfigurationScheduler
       */
      bool setProperty(char* key, char* value);

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
       * Used to indicate if a new configuration should be loaded by the controller
       * component.
       * \return  id of next configuration to be loaded or NULL if no configuration
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
       * \see AbstractController
       */
      virtual void signalTaskEvent(ProcessControlBlock* pcb);

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
      void calculateAssignTime(unsigned int nextConfiguration);

      /**
       * \brief Helper method to keep management structure uptodate
       */
      void updateUsedConfigurations(ProcessControlBlock* pcb);
  };

}
#endif /*HSCD_VPC_ROUNDROBINCONFSCHEDULER_H_*/
