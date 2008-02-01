#ifndef HSCD_VPC_LISTBINDER_H_
#define HSCD_VPC_LISTBINDER_H_

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_OfflineFile.h"
#include <systemc.h>
#include "hscd_vpc_Director.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_AbstractAllocator.h"
#include "hscd_vpc_OnlineAllocator.h"
namespace SystemC_VPC {

  /**
   * ListBinder is a implementation for a binding strategy.
   */
  class ListBinder : public DynamicBinder {

    private:
    
      int numberofcomp;
      
      //sc_time rctime[4];
      std::vector<sc_time> rctime;
      
      sc_time config_blocked_until;
  
    public:
       
      ListBinder::ListBinder();

      ListBinder::~ListBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock&, ReconfigurableComponent*) throw(UnknownBindingException);

      sc_time generate_sctime(std::string);
      
      void cleanstring(std::string*);
      
      void signalProcessEvent(ProcessControlBlock*, std::string);
      
      sc_time getSetupTime(ProcessControlBlock&);
  };


}

#endif //HSCD_VPC_LISTBINDER_H_
