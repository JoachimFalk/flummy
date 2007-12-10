#ifndef HSCD_VPC_OFFLINEBINDER_H_
#define HSCD_VPC_OFFLINEBINDER_H_

#include "hscd_vpc_AbstractBinder.h"

namespace SystemC_VPC {

  /**
   * OfflineBinder is a dummy implementation for a binding strategy.
   * It simply takes the first binding alternativ for binding a process to a component.
   */
  class OfflineBinder : public StaticBinder {

    public:
       
      OfflineBinder::OfflineBinder();

      OfflineBinder::~OfflineBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException);

      /**
       * \brief Implementation of ProcessEventListener::signalProcessEvent
       * Dummy implementation as OfflineBinder is not interested in task events
       * \sa ProcessEventListener
       */
      void signalProcessEvent(ProcessControlBlock* pcb, std::string compID);
  };


}

#endif //HSCD_VPC_OfflineBINDER_H_
