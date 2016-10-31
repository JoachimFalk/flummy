/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <systemcvpc/FastLink.hpp>
#include <systemcvpc/Director.hpp>

namespace SystemC_VPC{

  //
  void FastLink::compute( EventPair p ) const{
    //cerr << "FastLink " << this->process << ", " << this->func << endl;
    SystemC_VPC::Director::getInstance()
      .compute( *this, p, extraDelay );
    extraDelay = SC_ZERO_TIME;
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

  //
  ComponentId FastLink::getComponentId() const{
    return SystemC_VPC::Director::getInstance()
      .getComponent( *this )->getComponentId();
  }

  //
  void FastLink::addDelay(sc_time delay){
    extraDelay += delay;
  }
}
