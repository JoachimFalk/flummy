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

#ifndef HSCD_VPC_COMPONENTOBSERVER_H_
#define HSCD_VPC_COMPONENTOBSERVER_H_

#include "ComponentInfo.hpp"

namespace SystemC_VPC{

  class ComponentObserver
  {
  public:
    virtual ~ComponentObserver() {}

    // this callback function shall be called on component state changes
    virtual void notify(ComponentInfo *ci) = 0;
  };
}
#endif // HSCD_VPC_COMPONENTOBSERVER_H_
