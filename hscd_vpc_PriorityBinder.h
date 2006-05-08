#ifndef HSCD_VPC_PRIORITYBINDER_H_
#define HSCD_VPC_PRIORITYBINDER_H_

#include <map>
#include <string>

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_Controller.h"
#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC {

  class PriorityElement {
    
    public:

     virtual ~PriorityElement() {};

     
     
     /**
      * \brief Updates PriorityElement by selecting one possibilty out of the mapping informations
      * This method is used to update internal state of the PriorityElement.
      * Depending on the priority strategy it selects the most suitable MappingInformation
      * for the binding and returns it.
      * \param mIter enables iteration over possible binding settings
      * \return selected MappingInformation
      */
     virtual MappingInformation* addMappingData(MappingInformationIterator& mIter)=0;

     virtual void removeMappingData(MappingInformationIterator& mIter)=0; 
    
     virtual bool operator > (const PriorityElement& e)=0;

  };

  class PriorityElementFactory {

    public:

      virtual ~PriorityElementFactory(){}

      virtual PriorityElement* createInstance()=0;

  };
  
  class PriorityBinder : public DynamicBinder {

    private:

      // map of associated priority elements for a target
      std::map<std::string, PriorityElement*> pelems;

      PriorityElementFactory* factory;

    public:

      PriorityBinder(Controller* controller, MIMapper* miMapper, PriorityElementFactory* factory);

      ~PriorityBinder();

      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& task, AbstractComponent* comp) throw(UnknownBindingException);

      virtual void registerBinding(std::string src, std::string target);

      /**
       * \brief Implementation of TaskEventListener::signalTaskEvent
       */
      void signalTaskEvent(ProcessControlBlock* pcb);
  };
  
}

#endif //HSCD_VPC_PRIORITYBINDER_H_
