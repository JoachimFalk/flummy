#ifndef HSCD_VPC_CONFIGURATIONSCHEDULER_H_
#define HSCD_VPC_CONFIGURATIONSCHEDULER_H_

#include <systemc.h>

#include "hscd_vpc_AbstractAllocator.h"
#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_ReconfigurableComponent.h"

namespace SystemC_VPC {

  /**
   * \brief Implementation of base methods specified by AbstractAllocator
   * This class realizes basic function declared by AbstractAllocator, which
   * are used by all subclasses. If different behaviour is required, the specific
   * methods can be overloaded by the subclasses..
   * \see AbstractAllocator
   */
  class Allocator : public AbstractAllocator{

    private: 

      // associated controller instance
      AbstractController* controller;

      // true if controller uses kill to preempt configurations
      bool kill;

    protected:

      // time indicating next request wish
      sc_time* waitInterval;

      Allocator(AbstractController* controller);

    public:

      virtual ~Allocator();

      /**
       * \brief Returns time to wait until next notification of controller is needed
       * Returns time interval indicating when controlled component should invoke controller
       * next time.
       * \return time interval to wait or NULL if no time interval required
       */
      sc_time* getWaitInterval(ReconfigurableComponent* rc);

      /**
       * \brief Used to set Scheduler specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       * \sa AbstractAllocator
       */
      virtual bool setProperty(char* key, char* value);

      /**
       * \brief Setter to specify if controller should use "kill" by preemption
       */
      void setPreemptionStrategy(bool kill);

      /**
       * \brief Getter to determine which preemption mode is used
       */
      virtual bool deallocateByKill();  

      /**
       * \brief Signals to controller that managed component has been deallocated.
       * Used within controller to adapt scheduling to preemption of managed
       * component.
       * \note Does nothing intended for controllers not interested in preemption
       */
      virtual void signalDeallocation(bool kill, ReconfigurableComponent* rc);

      /**
       * \brief Signals to controller that managed component has been reallocated.
       * Used within controller to adapt scheduling to resuming of managed
       * component.
       * \note Does nothing intended for controllers not interested in resume
       */
      virtual void signalAllocation(ReconfigurableComponent* rc);

      virtual sc_time getSchedulingOverhead();

    protected:

      AbstractController& getController();

      /**
       * \brief Gets the currently conrtolled reconfigurable Component of instance
       */
      ReconfigurableComponent* getManagedComponent();
  };

}

#endif /*HSCD_VPC_CONFIGURATIONSCHEDULER_H_*/
