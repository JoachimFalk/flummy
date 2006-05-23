#ifndef HSCD_VPC_PRIORITYCONFSCHEDULER_H_
#define HSCD_VPC_PRIORITYCONFSCHEDULER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>


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
  class PriorityConfScheduler : public ConfigurationScheduler {

    private:

      int order_count;

      /**
       * Helper class to store configurations within priority list of controller.
       * Contains pointer to actual configuration and fifo entry for secundary strategy.
       */
      template<class C>
        class PriorityListElement{

          private:
            C contained;
            std::list<int> priorities;
            int fifo_order;

          public:
            PriorityListElement(C contained, int priority, int degree) : contained(contained), fifo_order(degree){
              this->priorities.push_back(priority);
              // finish list with -1
              this->priorities.push_back(-1);
            }

            void setContained(C* contained){
              this->contained = contained;
            }

            C getContained(){
              return this->contained;
            }

            void setFifoOrder(int degree){
              this->fifo_order = degree;
            }

            int getFifoOrder() const{
              return this->fifo_order;
            }

            void addPriority(int p){

              std::list<int>::iterator iter;

              // walk through list until priority in list is lesser than current to insert
              for(iter = this->priorities.begin(); iter != this->priorities.end() && *iter > p; iter++);

              this->priorities.insert(iter, p);

            }

            void removePriority(int p){

              std::list<int>::iterator iter;

              for(iter = this->priorities.begin(); iter != this->priorities.end(); iter++){
                if(*iter == p){
                  this->priorities.erase(iter);
                  break;
                }
              }

            }

            int getPriority() const{
              return this->priorities.front();
            }

            bool operator == (const C c){
              return this->contained == c;
            }

            bool operator > (const PriorityListElement& elem){
              if(this->getPriority() < elem.getPriority()){
                return true;
              }else
                if(this->getPriority() == elem.getPriority()
                    && this->getFifoOrder() < elem.getFifoOrder()){
                  return true;
                }
              return false;
            }

            bool operator < (const PriorityListElement& elem){
              if(this->getPriority() > elem.getPriority()){
                return true;
              }else
                if(this->getPriority() == elem.getPriority()
                    && this->getFifoOrder() > elem.getFifoOrder()){
                  return true;
                }
              return false;
            }
        };

      /**
       * Functor for comparing PriorityListElements
       */
      /*
         class PriorityComparator : public std::binary_function<PriorityListElement<Configuration* >, PriorityListElement<Configuration* >, bool>{
         public:
         bool operator ()(const PriorityListElement<Configuration* > & e1, const PriorityListElement<Configuration *> & e2) const{
         if(e1.getPriority() < e2.getPriority()){
         return true;
         }else
         if(e1.getPriority() == e2.getPriority()
         && e1.getFifoOrder() > e2.getFifoOrder()){
         return true;
         }
         return false;
         }

         };
         */
      // queue of tasks ready to be processed
      std::queue<ProcessControlBlock* > tasksToProcess;

      // queue containing order of configuration to be loaded in next "rounds"
      std::list<PriorityListElement<unsigned int> > nextConfigurations;

    public:

      PriorityConfScheduler(AbstractController* controller, MIMapper* miMapper);

      virtual ~PriorityConfScheduler();

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
       * \brief Used to signal finished tasks to the controller
       * \see TaskEventListener::signalTaskEvent
       */
      virtual void signalTaskEvent(ProcessControlBlock* pcb);

    private:

      /**
       * \brief Retrieve highest priority out of set of mapping possibilites
       * \param pid specifies the id of the task to determine priority for
       * \return highest priority
       */
      unsigned int getHighestPriority(int pid);
  };

}
#endif /*HSCD_VPC_PRIORITYCONFSCHEDULER_H_*/
