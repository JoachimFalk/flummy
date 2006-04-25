#include "hscd_vpc_SimpleBinder.h"

namespace SystemC_VPC {

  SimpleBinder::SimpleBinder() {}

  SimpleBinder::~SimpleBinder() {}

  std::string SimpleBinder::resolveBinding(std::string task, AbstractComponent* comp) throw(UnknownBindingException){
    AbstractBinding& b = this->getBinding(task);
    b.reset();
    if(b.hasNext()){
      return b.getNext();
    }

    std::string msg = "No target specified for "+ task +"->?";
    throw UnknownBindingException(msg);
  }

}
