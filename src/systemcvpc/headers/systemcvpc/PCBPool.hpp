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

#ifndef __INCLUDED__PCBPOOL__H__
#define __INCLUDED__PCBPOOL__H__

#include <map>
#include "FastLink.hpp"
#include "ProcessControlBlock.hpp"

namespace SystemC_VPC {
  typedef std::map<ProcessId, ProcessControlBlockPtr>  PCBPool;
}
#endif // __INCLUDED__PCBPOOL__H__
