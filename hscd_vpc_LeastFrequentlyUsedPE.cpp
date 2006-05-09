#include "hscd_vpc_LeastFrequentlyUsedPE.h"

namespace SystemC_VPC {

  unsigned int LeastFrequentlyUsedPE::global_id = 0;

  LeastFrequentlyUsedPE::LeastFrequentlyUsedPE(){
    this->id = LeastFrequentlyUsedPE::global_id++;
    this->bound_count = 0;
  }

  LeastFrequentlyUsedPE::~LeastFrequentlyUsedPE() {}

  MappingInformation* LeastFrequentlyUsedPE::addMappingData(MappingInformationIterator& mIter){
    this->bound_count++;
    // as its irrelevant which binding to take for strategy take first one
    if(mIter.hasNext()){
      return mIter.getNext();
    }
    return NULL;
  }

  void LeastFrequentlyUsedPE::removeMappingData(MappingInformationIterator& mIter){
    // do not decrease count as we want to have "history"
  }

  bool LeastFrequentlyUsedPE::operator > (const PriorityElement& e){

    try{
      // only able to compare priority elements of same type
      const LeastFrequentlyUsedPE& lfbPE = dynamic_cast<const LeastFrequentlyUsedPE&>(e);

#ifdef VPC_DEBUG
      std::cerr << "LFUPE> 'operator >' bound_count compare " << this->bound_count << " < " << lfbPE.bound_count << std::endl;
      std::cerr << "or id compare " << this->id << " < " << lfbPE.id << std::endl;
#endif //VPC_DEBUG
      
      if(this->bound_count < lfbPE.bound_count || (this->bound_count == lfbPE.bound_count && this->id < lfbPE.id)){
        return true;
      }
    }catch(std::bad_cast& e){
      // cannot compare types so this object is not higher prior
    }

    return false;
  }
 
  LFUPEFactory::LFUPEFactory() {}

  LFUPEFactory::~LFUPEFactory() {}

  PriorityElement* LFUPEFactory::createInstance(){
    
    return new LeastFrequentlyUsedPE();

  }

}

