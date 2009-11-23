#ifndef __INCLUDED__TASKPOOL__H__
#define __INCLUDED__TASKPOOL__H__

#include <systemcvpc/Pool.h>
#include <systemcvpc/FastLink.h>

namespace SystemC_VPC {
  class Task;
  typedef AssociativePrototypedPool<ProcessId, Task> TaskPool;

}
#endif // __INCLUDED__TASKPOOL__H__
