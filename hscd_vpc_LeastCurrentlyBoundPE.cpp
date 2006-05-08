#include "hscd_vpc_LeastCurrentlyBoundPE.h"

namespace SystemC_VPC {

  unsigned int LeastCurrentlyBoundPE::global_id = 0;

  LeastCurrentlyBoundPE::LeastCurrentlyBoundPE(){
    this->id = LeastCurrentlyBoundPE::global_id++;
    this->bound_count = 0;
  }

  LeastCurrentlyBoundPE::~LeastCurrentlyBoundPE() {}

  MappingInformation* LeastCurrentlyBoundPE::addMappingData(MappingInformationIterator& mIter){
    this->bound_count++;
    // as its irrelevant which binding to take for strategy take first one
    if(mIter.hasNext()){
      return mIter.getNext();
    }
    return NULL;
  }

  void LeastCurrentlyBoundPE::removeMappingData(MappingInformationIterator& mIter){
    if(this->bound_count > 0){
      this->bound_count--;
    }
  }

  bool LeastCurrentlyBoundPE::operator > (const PriorityElement& e){

    try{
      // only able to compare priority elements of same type
      const LeastCurrentlyBoundPE& lcbPE = dynamic_cast<const LeastCurrentlyBoundPE&>(e);

//#ifdef VPC_DEBUG
      std::cerr << "LCBPE> operator " << this->bound_count << " < " << lcbPE.bound_count << std::endl;
      std::cerr << "or " << this->id << " < " << lcbPE.id << std::endl;
//#endif //VPC_DEBUG
      
      if(this->bound_count < lcbPE.bound_count || this->id < lcbPE.id){
        return true;
      }
    }catch(std::bad_cast& e){
      // cannot compare types so this object is not higher prior
    }

    return false;
  }
 
  LCBPEFactory::LCBPEFactory() {}

  LCBPEFactory::~LCBPEFactory() {}

  PriorityElement* LCBPEFactory::createInstance(){
    
    return new LeastCurrentlyBoundPE();

  }

}

