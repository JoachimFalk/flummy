#ifndef HSCD_VPC_LPTBINDER_H_
#define HSCD_VPC_LPTBINDER_H_

#include <systemc.h>
#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_AbstractAllocator.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_OnlineAllocator.h"

namespace SystemC_VPC {

  /**
   * LPTBinder is a implementation for a binding strategy.
   */
  class LPTBinder : public DynamicBinder {

    private:
    
      int numberofcomp;
      
      std::vector<sc_time> rctime;
      
      sc_time config_blocked_until;
  
    public:
       
      LPTBinder::LPTBinder();

      LPTBinder::~LPTBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock&, ReconfigurableComponent*) throw(UnknownBindingException);

      sc_time generate_sctime(std::string);
      
      void cleanstring(std::string*);
      
      void signalProcessEvent(ProcessControlBlock*, std::string);
      
      Configuration* getConfiguration(ProcessControlBlock);
      
      sc_time getSetuptime(ProcessControlBlock);
      sc_time getRuntime(ProcessControlBlock);
  };


}

#endif //HSCD_VPC_LPTBinder_H_
