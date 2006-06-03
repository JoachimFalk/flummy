#ifndef HSCD_VPC_TASKEVENTLISTENER_
#define HSCD_VPC_TASKEVENTLISTENER_

#include <string>
#include "hscd_vpc_datatypes.h"

namespace SystemC_VPC{

  class ProcessControlBlock;
  class AbstractComponent;

  /**
   * TaskEventListener defines common interface used for communicating task state
   * through the control hierarchy within VPC framework.
   */
  class TaskEventListener{
  
  public:
    
    /**
     * \brief Signals task event to listener
     * Used by listend instance to notify listener about an
     * event which happened.
     * \param pcb points to the task which the notification corresponds to
     */
    virtual void signalTaskEvent(ProcessControlBlock* pcb, std::string compID)=0;
    
    /**
     * \note Not used yet
     */
    virtual void signalStateChanged(){};
    
  };

}

#endif /*HSCD_VPC_TASKEVENTLISTENER_*/
