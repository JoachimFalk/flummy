#ifndef IPREEMPTABLE_
#define IPREEMPTABLE_

#include <systemc.h>

namespace SystemC_VPC{
  
  class IDeallocatable{
  
    public:
      
      /**
       * \brief Preempts execution of component
       * Used to deallocate the current execution of a component.
       * Actual executed tasks are "stored" for later execution
       * or discarded depending on the parameter flag!
       * \param kill indicates if "hard" deallocation should be initiated
       * and currently registered task are killed without restoring
       * \return pointer to sc_time specifying time need for deallocation
       */
      virtual void deallocate(bool kill)=0;
        
      /**
       * \brief Resumes deallocated execution
       * Used to allocate execution of deallocated component.
       * \return pointer to sc_time specifying time need for resuming
       */
      virtual void allocate()=0;
      
      /**
       * \brief Gets time needed to deallocate a deallocatable object
       * Used to determine time needed to store current state of an
       * deallocatable instance, if restoring after deallocation is desired.
       * \return time needed to store actual state of an instance to deallocate
       */
      virtual sc_time timeToDeallocate()=0;
      
      /**
       * \brief Gets time needed to restore a deallocated object
       * Used to determine time needed to restore the state of an
       * deallocatable instance before its deallocation.
       * \return time needed to restore actual state of an instance to allocate
       */
      virtual sc_time timeToAllocate()=0;
      
  };

}
#endif /*IPREEMPTABLE_*/
