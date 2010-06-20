#ifndef HSCD_VPC_PROCESSEVENTLISTENER_
#define HSCD_VPC_PROCESSEVENTLISTENER_

#include "datatypes.hpp"
//#include "Task.hpp"

namespace SystemC_VPC{

  class Task;

  /**
   * ProcessEventListener defines common interface used for communicating task state
   * through the control hierarchy within VPC framework.
   */
  class ProcessEventListener{
  
  public:
    
    /**
     * \brief Signals task event to listener
     * Used by listend instance to notify listener about an
     * event which happened.
     * \param task points to the task which the notification corresponds to
     */
    virtual void signalProcessEvent(Task* task)=0;
    
    /**
     * \brief Virtual destructor for base classes
     * GCC complains if not present
     */
    virtual ~ProcessEventListener() {}      
  };

}

#endif /*HSCD_VPC_PROCESSEVENTLISTENER_*/
