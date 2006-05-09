#ifndef HSCD_VPC_LEASTFREQUENTLYBOUNDPE_H_
#define HSCD_VPC_LEASTFREQUENTLYBOUNDPE_H_

#include "hscd_vpc_PriorityBinder.h"

namespace SystemC_VPC {

  /**
   * \brief Specialized PriorityElement taking into account total_bound_count
   * This class is used to enable fair share of binding possibilities over time.
   * It uses Least Frequently Used semantics for priority decision, that means
   * the least used binding is the most prior one.
   */
  class LeastFrequentlyUsedPE : public PriorityElement {

    private:

      static unsigned int global_id;
      
      // id of element used a secondary compare strategy
      unsigned int id;

      unsigned int bound_count;
    public:

      LeastFrequentlyUsedPE();
      
      virtual ~LeastFrequentlyUsedPE();

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

      unsigned int getUsedCount() const;
  };
 
  class LFUPEFactory : public PriorityElementFactory {

    public:

      LFUPEFactory();

      ~LFUPEFactory(); 

      PriorityElement* createInstance();

  };
}

#endif //LEASTFREQUENTLYBOUNDPE_H_
