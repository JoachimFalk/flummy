#ifndef HSCD_VPC_ARBINDER_H_
#define HSCD_VPC_ARBINDER_H_

#include <map>
#include <string>

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_Controller.h"
#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC {

  /**
   * \brief LocalBinder (Avoid Reconfiguraton) considering reconfiguration overhead in current and successing level
   * Implementation of localized Binder considering binding targets. If task is bound
   * to reconfigurable component it is checked if required configuration is available.
   */
  class ARBinder : public DynamicBinder {

    private:
    
      // multimap storing information: comp->(task, #tasks)
      // to reflect already taken resolutions
      std::multimap<std::string, std::pair<std::string, int> > boundTo;
      std::map<std::string, std::string> decisions;

    public:
      
      ARBinder(Controller* controller, MIMapper* miMapper);

      ~ARBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException);

      /**
       * \brief Implementation of TaskEventListener::signalTaskEvent
       */
      void signalTaskEvent(ProcessControlBlock* pcb, std::string compID);

    private:

      bool reconfRequired(ProcessControlBlock& task, std::string target, ReconfigurableComponent* comp);

  };
  
}

#endif //HSCD_VPC_ARBINDER_H_
