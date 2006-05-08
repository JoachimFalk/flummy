#include "hscd_vpc_SimpleBinder.h"

namespace SystemC_VPC {

  SimpleBinder::SimpleBinder(Controller* controller, MIMapper* miMapper) : StaticBinder(controller, miMapper) {}

  SimpleBinder::~SimpleBinder() {}

  std::pair<std::string, MappingInformation* > SimpleBinder::performBinding(ProcessControlBlock& task, AbstractComponent* comp) throw(UnknownBindingException){
    AbstractBinding& b = this->getBinding(task.getName());
    
    b.reset();
    if(b.hasNext()){
      std::string comp = b.getNext();
      MIMapper& mapper = this->getMIMapper();
      MappingInformationIterator* iter = mapper.getMappingInformationIterator(comp);
      if(iter->hasNext()){
        MappingInformation* mInfo = iter->getNext();
        delete iter;
        return std::pair<std::string, MappingInformation*>(comp, mInfo);
      }else{
        // also free iterator
        delete iter;
      }
    }

    std::string msg = "No target specified for "+ task.getName() +"->?";
    throw UnknownBindingException(msg);
  }

}
