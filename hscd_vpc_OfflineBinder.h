#ifndef HSCD_VPC_OFFLINEBINDER_H_
#define HSCD_VPC_OFFLINEBINDER_H_

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_OfflineFile.h"
#include "hscd_vpc_StringParser.h"

namespace SystemC_VPC {

  /**
   * OfflineBinder is a implementation for a binding strategy.
   * It takes the binding decision from the schedule config file.
   */
  class OfflineBinder : public StaticBinder {

    private:
      char *OfflineFileName;
  
    public:
       
      OfflineBinder::OfflineBinder(char*);

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
