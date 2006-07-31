#ifndef HSCD_VPC_SIMPLECBINDER_H_
#define HSCD_VPC_SIMPLEBINDER_H_

#include "hscd_vpc_AbstractBinder.h"

namespace SystemC_VPC {

  /**
   * SimpleBinder is a dummy implementation for a binding strategy.
   * It simply takes the first binding alternativ for binding a process to a component.
   */
  class SimpleBinder : public StaticBinder {

    public:
       
      SimpleBinder::SimpleBinder();

      SimpleBinder::~SimpleBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException);

      /**
       * \brief Implementation of ProcessEventListener::signalProcessEvent
       * Dummy implementation as SimpleBinder is not interested in task events
       * \sa ProcessEventListener
       */
      void signalProcessEvent(ProcessControlBlock* pcb, std::string compID);
  };


}

#endif //HSCD_VPC_SIMPLEBINDER_H_
