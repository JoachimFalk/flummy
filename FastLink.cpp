#include <systemcvpc/FastLink.hpp>
#include <systemcvpc/Director.hpp>

namespace SystemC_VPC{

  //
  void FastLink::compute( EventPair p ) const{
    //cerr << "FastLink " << this->process << ", " << this->func << endl;
    SystemC_VPC::Director::getInstance()
      .compute( *this, p );
  }

  //
  void FastLink::write( size_t quantum, EventPair p ) const{
    SystemC_VPC::Director::getInstance()
      .write( *this, quantum, p );
  }

  //
  void FastLink::read( size_t quantum, EventPair p ) const{
    SystemC_VPC::Director::getInstance()
      .read( *this, quantum, p );
  }
}
