#ifndef HSCD_VPC_TASKEVENTLISTENER_
#define HSCD_VPC_TASKEVENTLISTENER_

#include "hscd_vpc_datatypes.h"

namespace SystemC_VPC{
  
  /**
   * TaskEventListener defines common interface used for communicating task state
   * through the control hierarchy within VPC framework.
   */
  class TaskEventListener{
  
  public:
    
    /**
     * \brief Signals task event to listener
     * Used by listend instance to notify listener about an
     * event which happend.
     * \param pcb points to the task which the notifikation corresponds to
     */
    virtual void signalTaskEvent(p_struct* pcb)=0;
    
    /**
     * \note Not used yet
     */
    virtual void signalStateChanged(){};
    
  };

}

#endif /*HSCD_VPC_TASKEVENTLISTENER_*/
