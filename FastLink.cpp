#include "FastLink.h"
#include "hscd_vpc_Director.h"

namespace SystemC_VPC{

  void FastLink::compute( EventPair p ) const{
    //cerr << "FastLink " << this->process << ", " << this->func << endl;
    SystemC_VPC::Director::getInstance()
      .compute( *this, p );
  }

}
