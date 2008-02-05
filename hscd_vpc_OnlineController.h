#ifndef HSCD_VPC_ONLINECONTROLLER_H_
#define HSCD_VPC_ONLINECONTROLLER_H_

#include <systemc.h>
#include <string>
#include <map>
#include <queue>
#include <vector>

#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_AbstractAllocator.h"
#include "hscd_vpc_Allocator.h"
#include "hscd_vpc_Controller.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_OnlineController.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_VPCBuilder.h"

namespace SystemC_VPC {

  /**
   * OnlineController is a implementation for a binding strategy.
   */
  class OnlineController : public DynamicBinder , public Allocator {

    private:
    
      int numberofcomp;
      
      std::vector<sc_time> rctime;
      
      sc_time config_blocked_until;
  
      // queue of waiting processes to be executed
      std::deque<std::pair<ProcessControlBlock*, unsigned int> > readyTasks;
      // map of running processes
      std::map<int, ProcessControlBlock* > runningTasks;
      // queue of processes ready to be processed
      std::queue<ProcessControlBlock* > tasksToProcess;

      // pointer to next configuration to be loaded
      unsigned int nextConfiguration;

    public:
       
      OnlineController::OnlineController();
      OnlineController::OnlineController(AbstractController*);
      
      OnlineController::~OnlineController();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock&, ReconfigurableComponent*) throw(UnknownBindingException);

      sc_time generate_sctime(std::string);
      
      void cleanstring(std::string*);
      
      /**
       * \see ProcessEventListener
       */
      virtual void signalProcessEvent(ProcessControlBlock*, std::string);

      /**
       * \brief Updates management structures for performing schedule decisions
       * This method is used to initialize and set up all necessary data for a new "round" of
       * scheduling.
       * \param newTasks refers to new process to be scheduled
       * \param config refers to the required configuration which has to be scheduled
       */
      virtual void addProcessToSchedule(ProcessControlBlock*, unsigned int, ReconfigurableComponent*);

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
      
      sc_time getSchedulingOverhead();
      
      void setBlockedTime(sc_time);

      Configuration* getConfiguration(ProcessControlBlock);
      
      sc_time getSetuptime(ProcessControlBlock);
      
  };

} //end of Namespace SystemC_VPC

#endif
