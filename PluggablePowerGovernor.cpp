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

#include <systemcvpc/PluggablePowerGovernor.hpp>

namespace SystemC_VPC{
  GenericParameter::~GenericParameter() {}

  PowerModeParameter::PowerModeParameter(PowerMode *mode) : powerMode(mode){}
  PowerModeParameter::~PowerModeParameter() {}
}
