#ifndef HSCD_VPC_PRIORITYBINDER_H_
#define HSCD_VPC_PRIORITYBINDER_H_

#include <map>
#include <string>

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_Controller.h"
#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC {

  /**
   * \brief Basic PriorityElement interface specifying necessary methods for usage with PriorityBinder
   */
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

     /**
      * \brief Notifies PriorityElement if task has been removed from associated target
      * Designed to update internal state of PriorityElement on task "removal" if necessary.
      * \param mIter enables iteration over possible binding settings
      */
     virtual void removeMappingData(MappingInformationIterator& mIter)=0; 
    
     virtual bool operator > (const PriorityElement& e)=0;

  };

  /**
   * \brief Factory interface used to instantiate PriortyElement instances
   */
  class PriorityElementFactory {

    public:

      virtual ~PriorityElementFactory(){}

      virtual PriorityElement* createInstance()=0;

  };

  /**
   * \brief Priority based binder for selection of multiple binding possibilities
   * This class uses PriorityElements associated with the binding targets to perform
   * its binding strategies. The actual state of each target priority element is taken
   * into account and update respectivly to the selected binding.  
   */
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
      void signalTaskEvent(ProcessControlBlock* pcb, std::string CompID);
  };
  
}

#endif //HSCD_VPC_PRIORITYBINDER_H_
