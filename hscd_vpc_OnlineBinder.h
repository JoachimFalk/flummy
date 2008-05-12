#ifndef HSCD_VPC_ONLINEBINDER_H_
#define HSCD_VPC_ONLINEBINDER_H_

#include <systemc.h>
#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_AbstractAllocator.h"
#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_OnlineAllocator.h"

namespace SystemC_VPC {

  /**
   * OnlineBinder is a implementation of online scheduling binding strategy
   */
  class OnlineBinder : public DynamicBinder {

    private:
    
      int numberofcomp;

      char* algorithm;      

      class timesTable_entry{
      public:
        sc_time time;
        int recomponentnumber;
        timesTable_entry(sc_time time, int recomponentnumber) :
          time(time),
          recomponentnumber(recomponentnumber){}
        timesTable_entry() :
          time(SC_ZERO_TIME),
          recomponentnumber(0) {}
        bool operator<(const timesTable_entry& other) const {
          return time < other.time;
        }
        bool operator<=(const timesTable_entry& other) const {
          return time <= other.time;
        }
      };
      
      vector<timesTable_entry> timesTable;
      
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
       
      OnlineBinder::OnlineBinder(char*);

      OnlineBinder::~OnlineBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock&, ReconfigurableComponent*) throw(UnknownBindingException);
    
      void signalProcessEvent(ProcessControlBlock*, std::string);
      
      Configuration* getConfiguration(ProcessControlBlock);
      
      sc_time getSetuptime(ProcessControlBlock);
      sc_time getRuntime(ProcessControlBlock);
  };

}

#endif //HSCD_VPC_ONLINEBINDER_H_
