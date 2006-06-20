#ifndef HSCD_VPC_RRECONFSCHEDULER_H_
#define HSCD_VPC_RRECONFSCHEDULER_H_

#include <deque>
#include <map>

#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_Allocator.h"

namespace SystemC_VPC {

  /**
   * \brief helper structure to contain scheduling information
   */
  class RREConfElement {

    private:

      unsigned int confID;
      std::deque<ProcessControlBlock* > runningTasks;
      std::deque<ProcessControlBlock* > waitingTasks;
      
    public:

      /**
       * \brief default constructor
       * \param confID specifies associated Configuration
       */
      RREConfElement(unsigned int confID);

      /**
       * \brief default destructor
       */
      ~RREConfElement();

      /**
       * \brief Simple Getter
       */
      unsigned int getID();
      
      /**
       * \brief adds new task to waitingQueue of management element 
       * \param pcb refers to ProcessControlBlock to add
       */
      void addTaskWQueue(ProcessControlBlock* pcb);

      /**
       * \brief moves task to runningQueue
       * Used to get next task in waiting queue and to move it to running
       * queue.
       * \return next task in waiting queue
       */
      ProcessControlBlock* processTask();
     
      /**
       * \brief indicates if there are still task to be processed
       */
      bool hasWaitingTasks();
     
      /**
       * \brief used to deque task out of waiting list without adding to running tasks
       * \return pcb referring to waiting task
       */
      ProcessControlBlock* getWaitingTask();

      /**
       * \brief removes task from management element
       * \param pcb refers to ProcessControlBlock to remove
       */
      void removeTask(ProcessControlBlock* pcb);
     
      /**
       * \brief used to determine if current element is higher prior than given
       * \return true if current element has higher priority else false
       */
      bool operator< (const RREConfElement& elem); 

      /**
       * \brief used to determine if configuration still in use
       */
      int getAssignedTaskCount();

      /**
       * \brief determine execution time of all associated running tasks
       */
      sc_time RREConfElement::getExecutionSum();
  };
  
  /**
   * \brief Adapted RoundRobin-based Binder trying to minimize reconfiguration latency by still ensuring fairness
   * Uses a given parameter alpha giving a proportion between reconfiguration time and usage time of a  configuration.
   * Factor 0.5 of alpha means that a configuration should run minimal as long as it takes to load and store it.
   * If there are pending configurations to schedule and current configuration is no longer needed reconfiguration will
   * take place although timeslice may not be elapsed!
   * \note Allocation-Strategy is based on local view
   */
  class RREAllocator : public Allocator {

    private:

      int taskCount; //< count running and waiting tasks -> increased if new ones are added decrease if task are signalled
      double alpha;
      // time of assignment
      sc_time lastassign;
      // time selected configuration has been activ
      sc_time activTime;
      std::map<int, RREConfElement* > elems;
      RREConfElement* selected;

    public:

      RREAllocator(AbstractController* ctrl, double alpah=0.5);

      ~RREAllocator();

      /**
       * \brief Updates management structures for performing schedule decisions
       * This method is used to initialize and set up all necessary data for a new "round" of
       * scheduling. 
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      void addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc); 

      /**
       * \brief Realizes scheduling decision for tasks to be forwarded to configurations
       * This method is used to perform scheduling decision for tasks and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller.
       * \param newTasks refers to new task to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */ 
      void performSchedule(ReconfigurableComponent* rc);

      /**
       * \brief Returns next configuration to be loaded
       * Used to indicate if a new configuration should be loaded by the controller
       * component.
       * \return id of next configuration to be loaded or 0 if no configuration
       * is selected up to now.
       */
      unsigned int getNextConfiguration(ReconfigurableComponent* rc);

      /**
       * \brief Indicates if controller still can forward tasks
       * \return TRUE if there are still task to be forwarded else FALSE
       */
      bool hasTaskToProcess(ReconfigurableComponent* rc);

      /**
       * \brief Returns next task to be forwarded
       * This method should only be called after calling hasTaskToProcess
       * to ensure that there are still existing task to process.
       * \return pair containing ProcessControlBlock of task and requested function
       * to be simulated.
       */
      ProcessControlBlock* getNextTask(ReconfigurableComponent* rc);

      /**
       * \brief Used to set Scheduler specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       */
      bool setProperty(char* key, char* value);


      /**
       * \brief Signals to controller that managed component has been preempted.
       * Used within controller to adapt scheduling to preemption of managed
       * component.
       * \param kill indicates if preemption happend with kill flag
       */
      void signalPreemption(bool kill, ReconfigurableComponent* rc);

      /**
       * \brief Signals to controller that managed component has been resumed.
       * Used within controller to adapt scheduling to resuming of managed
       * component.
       */
      void signalResume(ReconfigurableComponent* rc);

      void signalTaskEvent(ProcessControlBlock* pcb, std::string compID);

    private:

      /**
       * \brief determine if currently scheduled configuration should be killed
       * This method is internally used to determine if actual scheduled configuration
       * is killed by displacement
       * \return true if kill should be used
       */
      bool killConfiguration(RREConfElement* elem, ReconfigurableComponent* rc);

      /**
       * \brief update internal attributes
       * Used to updateWaitInterval for controlled instance
       */
      void setUpInitialParams(RREConfElement* next, ReconfigurableComponent* rc);

  };
  
}

#endif //HSCD_VPC_RRECONFSCHEDULER_H_
