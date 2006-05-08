#ifndef HSCD_VPC_LEASTCURRENTYLBOUNDPE_H_
#define HSCD_VPC_LEASTCURRENTLYBOUNDPE_H_

#include "hscd_vpc_PriorityBinder.h"

namespace SystemC_VPC {

  class LeastCurrentlyBoundPE : public PriorityElement {

    private:

      static unsigned int global_id;
      
      // id of element used a secondary compare strategy
      unsigned int id;

      unsigned int bound_count;

    public:

      LeastCurrentlyBoundPE();
      
      virtual ~LeastCurrentlyBoundPE();

      /**
       * \brief used to update internal state of PriorityElement
       * This method is used to update internal state of the PriorityElement.
       * Depending on the priority strategy it selects the most suitable MappingInformation
       * for the binding and returns it.
       * \param mIter enables iteration over possible binding settings
       * \return selected MappingInformation
       */
      MappingInformation* addMappingData(MappingInformationIterator& mIter);

      void removeMappingData(MappingInformationIterator& mIter);

      bool operator > (const PriorityElement& e);

      unsigned int getID() const;

      unsigned int getBoundCount() const;
  };
 
  class LCBPEFactory : public PriorityElementFactory {

    public:

      LCBPEFactory();

      ~LCBPEFactory(); 

      PriorityElement* createInstance();

  };
}

#endif //LEASTCURRENTLYBOUNDPE_H_
