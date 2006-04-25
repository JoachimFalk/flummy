#ifndef HSCD_VPC_SIMPLECBINDER_H_
#define HSCD_VPC_SIMPLEBINDER_H_

#include "hscd_vpc_AbstractBinder.h"

namespace SystemC_VPC {

  class SimpleBinder : public StaticBinder {

    public:

      SimpleBinder::SimpleBinder();

      SimpleBinder::~SimpleBinder();

      std::string resolveBinding(std::string task, AbstractComponent* comp) throw(UnknownBindingException);

  };


}

#endif //HSCD_VPC_SIMPLEBINDER_H_
