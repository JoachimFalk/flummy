#ifndef HSCD_VPC_OFFLINEALLOCATOR_H_
#define HSCD_VPC_OFFLINEALLOCATOR_H_

#include <systemc.h>
#include <string>
#include <map>
#include <queue>
#include <vector>

#include "hscd_vpc_Allocator.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_OfflineFile.h"
#include "hscd_vpc_StringParser.h"

namespace SystemC_VPC{

  /** Mission: Offline Scheduler
   * 
   */
  class OfflineAllocator : public Allocator {

    private:

      AbstractController* controller;
      
      // list of times, processes, configs => timesTable
      
      class timesTable_entry{
      public:
        sc_time time;
        string taskname;
        string recomponentname;
        timesTable_entry(sc_time time, string taskname, string recomponentname) :
          time(time),
          taskname(taskname),
          recomponentname(recomponentname){}
        timesTable_entry() :
          time(SC_ZERO_TIME),
          taskname(""),
          recomponentname("") {}
      };

      struct timesTable_compare{
      bool operator()(const timesTable_entry& pqe1, const timesTable_entry& pqe2) const
        {
          sc_time p1=pqe1.time;
          sc_time p2=pqe2.time;
          if (p1 > p2)
            return true;
          else
            return false;
        }
      };
      
      priority_queue<timesTable_entry,vector<timesTable_entry>,timesTable_compare> timesTable;
    
      // tasks added to schedule
      std::vector<std::pair<ProcessControlBlock*,unsigned int> > tasks;
      
      // internal task and config status
      ProcessControlBlock* currTask;
      unsigned int reqConfig;
      
      // pointer to next configuration to be loaded
      unsigned int nextConfiguration;
      
    public:

      OfflineAllocator(AbstractController* controller);

      virtual ~OfflineAllocator();

     /**
      * \brief Initializes Controller after Component has been set
      * This method is used to initialize the controller after a component has been set.
      */
      virtual void initController(char*);
            
      /**
       * \brief Updates management structures for performing schedule decisions
       * This method is used to initialize and set up all necessary data for a new "round" of
       * scheduling.
       * \param newTasks refers to new process to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void addProcessToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc);

      /**
       * \brief Realizes scheduling decision for processes to be forwarded to configurations
       * This method is used to perform scheduling decision for processes and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller.
       * \param newTasks refers to new process to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void performSchedule(ReconfigurableComponent* rc);

      /**
       * \brief Realizes scheduling decision for processes to be forwarded to configurations
       * This method is used to perform scheduling decision for processes and within this context
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
      virtual unsigned int getNextConfiguration(ReconfigurableComponent* rc);

      /**
       * \brief Indicates if controller still can forward processes
       * \return TRUE if there are still process to be forwarded else FALSE
       */
      virtual bool hasProcessToDispatch(ReconfigurableComponent* rc);

      /**
       * \brief Returns next process to be forwarded
       * This method should only be called after calling hasProcessToDispatch
       * to ensure that there are still existing process to process.
       * \return pair containing ProcessControlBlock of process and requested function
       * to be simulated.
       */
      virtual ProcessControlBlock* getNextProcess(ReconfigurableComponent* rc);

      /**
       * \brief Signals if a configuration has to be reactived by controlled component
       * \param config points to configuration which should be reactivated.
       */ 
      bool needToReactivateConfiguration(Configuration* config, ReconfigurableComponent* rc);

      /**
       * \see ProcessEventListener
       */
      virtual void signalProcessEvent(ProcessControlBlock* pcb, std::string compID);

      /**
       * \brief Implementation of AbstractAllocator::signalDeallocation
       * \see AbstractAllocator
       */
      void signalDeallocation(bool kill, ReconfigurableComponent* rc);

      /**
       * \brief Signals always true as configuration is only switched if all process have finished
       * or have been aborted
       * So this may seem strange as FCFS is non preemptiv, but as it takes care that no more processes running, this behaviour is OK.
       */
      bool deallocateByKill(){
        return true;
      } 
  };

}
#endif /*HSCD_VPC_OFFLINEALLOCATOR_H_*/
