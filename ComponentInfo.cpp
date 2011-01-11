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

#include <systemcvpc/ComponentInfo.hpp>

const SystemC_VPC::ComponentState SystemC_VPC::ComponentState::IDLE    = 0;
const SystemC_VPC::ComponentState SystemC_VPC::ComponentState::RUNNING = 1;
const SystemC_VPC::ComponentState SystemC_VPC::ComponentState::STALLED = 2;
