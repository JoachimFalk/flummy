#ifndef HSCD_VPC_LISTBINDER_H_
#define HSCD_VPC_LISTBINDER_H_

#include <systemc.h>
#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_AbstractAllocator.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_OnlineAllocator.h"

namespace SystemC_VPC {

  /**
   * ListBinder is a implementation for a binding strategy.
   */
  class ListBinder : public DynamicBinder {

    private:
    
      int numberofcomp;
      
      std::vector<sc_time> rctime;
      
      sc_time config_blocked_until;

      class slotTable_entry{
      public:
        int recomponentnumber;
        std::string recomponentname;
        slotTable_entry(int recomponentnumber, string recomponentname) :
          recomponentnumber(recomponentnumber),
          recomponentname(recomponentname){}
        slotTable_entry() :
          recomponentnumber(0),
          recomponentname("") {}
      };
       
      vector<slotTable_entry> slotTable;

  
    public:
       
      ListBinder::ListBinder();

      ListBinder::~ListBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock&, ReconfigurableComponent*) throw(UnknownBindingException);
    
      void signalProcessEvent(ProcessControlBlock*, std::string);
      
      Configuration* getConfiguration(ProcessControlBlock);
      
      sc_time getSetuptime(ProcessControlBlock);
      sc_time getRuntime(ProcessControlBlock);
  };


}

#endif //HSCD_VPC_LISTBINDER_H_
