#ifndef HSCD_VPC_CONTROLLER_H_
#define HSCD_VPC_CONTROLLER_H_

#include <string>
#include <map>
#include <set>
#include <queue>
#include <deque>
#include <vector>

#include "hscd_vpc_AbstractController.h"

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_AbstractConfigurationMapper.h"
#include "hscd_vpc_AbstractAllocator.h"

#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC {

  /**
   * \brief Implementation of base methods specified by AbstractController
   * This class realizes basic function declared by AbstractController, which
   * are used by all subclasses. If different behaviour is required, the specific
   * methods can be overloaded by the subclasses..
   * \see AbstractController
   */
  class Controller : public AbstractController{

    private:

      char controllerName [VPC_MAX_STRING_LENGTH];

      // controlled component of instance
      ReconfigurableComponent* managedComponent;

      // refers to binder to resolve binding of tasks
      AbstractBinder* binder;

      // refers to mapper to resolve "binding" of components to configuration
      AbstractConfigurationMapper* mapper;

      // refers to configuration scheduler to manage dynamic allocation
      AbstractAllocator* scheduler;

      // map storing made decisions
      std::map<int, Decision> decisions;

    protected:

      // time indicating next request wish
      sc_time* waitInterval;

    public:

      Controller(const char* name);

      virtual ~Controller();

      /**
       * \brief Getter for controller name
       */
      char* getName();

      /**
       * \brief Getter for binder instance
       */
      AbstractBinder* getBinder();

      /**
       * \brief Sets binder of controller instance
       */
      void setBinder(AbstractBinder* binder);

      /**
       * \brief Getter for mapper instance
       */
      AbstractConfigurationMapper* getConfigurationMapper();

      /**
       * \brief Sets configuration mapper of controller instance
       */
      void setConfigurationMapper(AbstractConfigurationMapper* mapper);

      /**
       * \brief Getter for scheduler instance
       */
      AbstractAllocator* getConfigurationScheduler();

      /**
       * \brief Sets configuration scheduler of controller instance
       */
      void setConfigurationScheduler(AbstractAllocator* scheduler);

      /**
       * \brief Sets the currently controlled reconfigurable Component of instance
       */
      void setManagedComponent(ReconfigurableComponent* managedComponent);

      /**
       * \brief Gets the currently controlled reconfigurable Component of instance
       */
      ReconfigurableComponent* getManagedComponent();

      /**
       * \brief Realizes scheduling decision for tasks to be forwarded to configurations
       * This method is used to perform scheduling decision for tasks and within this context
       * their corresponding configurationgs depending on the strategie of the different
       * controller. It is used to initialize and set up all necessary data for a new "round" of
       * scheduling. 
       * \param newTasks refers to a queue of pcb to be scheduled
       * \sa AbstractController
       */
      virtual void addTasksToSchedule(std::deque<ProcessControlBlock* >& newTasks, ReconfigurableComponent* rc);

      virtual void performSchedule(ReconfigurableComponent* rc);

      /**
       * \brief Returns time to wait until next notification of controller is needed
       * Returns time interval indicating when controlled component should invoke controller
       * next time.
       * \return time interval to wait or NULL if no time interval required
       */
      sc_time* getWaitInterval(ReconfigurableComponent* rc);

      /**
       * \brief Register component to Director
       * Used to register a component to the Director for
       * later computation of task on it. The components name
       * is used as identifier for it.
       * \param comp points to component instance to be registered
       */
      virtual void registerComponent(AbstractComponent* comp);

      /**
       * \brief Registers mapping between task and component to Director
       * \param taskName specifies name of task
       * \param compName specifies name of component
       * \param mInfo specifies associated mapping information
       */
      //virtual void registerMapping(const char* taskName, const char* compName, MappingInformation* mInfo, AbstractComponent* c);

      /**
       * \brief Returns mapped component for a given task
       * \param task specifies the task to get component for
       * \return pointer to AbstractComponent refering to mapped component
       */
      virtual AbstractComponent* getMappedComponent(ProcessControlBlock* task, ReconfigurableComponent* rc);

      virtual bool hasTaskToProcess(ReconfigurableComponent* rc);

      virtual ProcessControlBlock* getNextTask(ReconfigurableComponent* rc);

      virtual unsigned int getNextConfiguration(ReconfigurableComponent* rc);

      /**
       * \brief Used to set controller specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       */
      virtual void setProperty(char* key, char* value);

      /**
       * \brief Getter to determine which preemption mode is used
       */
      virtual bool preemptByKill();  

      /**
       * \brief Signals to controller that managed component has been preempted.
       * Used within controller to adapt scheduling to preemption of managed
       * component.
       * \param kill indicates if preemption happend with kill flag
       * \note Does nothing intended for controllers not interested in preemption
       */
      virtual void signalPreemption(bool kill, ReconfigurableComponent* rc);

      /**
       * \brief Signals to controller that managed component has been resumed.
       * Used within controller to adapt scheduling to resuming of managed
       * component.
       * \note Does nothing intended for controllers not interested in resume
       */
      virtual void signalResume(ReconfigurableComponent* rc);

      /**
       * \brief 
       * \sa AbstractController
       */
      virtual Decision getDecision(int pid, ReconfigurableComponent* rc);

      virtual void signalTaskEvent(ProcessControlBlock* pcb, std::string compID);

  };

}

#endif /*HSCD_VPC_CONTROLLER_H_*/
