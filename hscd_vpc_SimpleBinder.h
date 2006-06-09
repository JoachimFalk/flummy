#ifndef HSCD_VPC_SIMPLECBINDER_H_
#define HSCD_VPC_SIMPLEBINDER_H_

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_Controller.h"

namespace SystemC_VPC {

  class SimpleBinder : public StaticBinder {

    public:
       
      SimpleBinder::SimpleBinder(Controller* controller, MIMapper* miMapper);

      SimpleBinder::~SimpleBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException);

      /**
       * \brief Implementation of TaskEventListener::signalTaskEvent
       * Dummy implementation as SimpleBinder is not interested in task events
       */
      void signalTaskEvent(ProcessControlBlock* pcb, std::string compID) {}
  };


}

#endif //HSCD_VPC_SIMPLEBINDER_H_
