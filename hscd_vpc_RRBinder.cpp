#include "hscd_vpc_RRBinder.h"
#include "hscd_vpc_MIMapper.h"

namespace SystemC_VPC {
  
  RRBinder::RRBinder(Controller* controller, MIMapper* miMapper) : DynamicBinder(controller, miMapper) {}

  RRBinder::~RRBinder() {}

  std::pair<std::string, MappingInformation* > RRBinder::performBinding(ProcessControlBlock& pcb, AbstractComponent* comp)
    throw(UnknownBindingException) {
      
    AbstractBinding& binding = this->getBinding(pcb.getName());

    // reset binding iterator if end of possibilites reached
    if(!binding.hasNext()){
      binding.reset();
    }
    
    // check if binding possibility exists
    if(binding.hasNext()){
      std::string comp = binding.getNext();
      MIMapper& mapper = this->getMIMapper();
      MappingInformationIterator* iter = mapper.getMappingInformationIterator(comp);
      if(iter->hasNext()){
        MappingInformation* mInfo = iter->getNext();
        delete iter;
        return std::pair<std::string, MappingInformation* >(comp, mInfo);
      }else{
        // also free iter
        delete iter;
      }
    }

    std::string msg = "No binding possibility given for "+ pcb.getName() +"->?";
    throw UnknownBindingException(msg);
  }

  void RRBinder::signalTaskEvent(ProcessControlBlock* pcb, std::string CompID){
    // do nothing
  }

}
