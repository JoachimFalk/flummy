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

#include <systemcvpc/RouteImpl.hpp>
namespace SystemC_VPC {
  size_t Route::createRouteId(){
    static size_t instanceCounter = 0;
    return ++instanceCounter; // start from 1
  }
}
