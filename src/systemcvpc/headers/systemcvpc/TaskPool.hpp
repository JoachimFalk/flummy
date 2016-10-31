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

#ifndef __INCLUDED__TASKPOOL__H__
#define __INCLUDED__TASKPOOL__H__

#include <systemcvpc/Pool.hpp>
#include <systemcvpc/FastLink.hpp>

namespace SystemC_VPC {
  class Task;
  typedef AssociativePrototypedPool<ProcessId, Task> TaskPool;

}
#endif // __INCLUDED__TASKPOOL__H__
