#ifndef __INCLUDED__TASKPOOL__H__
#define __INCLUDED__TASKPOOL__H__

#include "Pool.h"
#include "FastLink.h"

namespace SystemC_VPC {
  class Task;
  typedef AssociativePrototypedPool<ProcessId, Task> TaskPool;

}
#endif // __INCLUDED__TASKPOOL__H__
