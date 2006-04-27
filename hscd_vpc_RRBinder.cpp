#include "hscd_vpc_RRBinder.h"

namespace SystemC_VPC {
  
  RRBinder::RRBinder() {}

  RRBinder::~RRBinder() {}

  std::string RRBinder::resolveBinding(std::string task, AbstractComponent* comp) throw(UnknownBindingException) {
    AbstractBinding& binding = this->getBinding(task);

    // reset binding iterator if end of possibilites reached
    if(!binding.hasNext()){
      binding.reset();
    }
    
    // check if binding possibility exists
    if(binding.hasNext()){
      return binding.getNext();
    }

    std::string msg = "No binding possibility given for "+ task +"->?";
    throw UnknownBindingException(msg);
  }

}
