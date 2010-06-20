#ifndef __INCLUDED__TASKPOOL__H__
#define __INCLUDED__TASKPOOL__H__

#include <systemcvpc/Pool.hpp>
#include <systemcvpc/FastLink.hpp>

namespace SystemC_VPC {
  class Task;
  typedef AssociativePrototypedPool<ProcessId, Task> TaskPool;

}
#endif // __INCLUDED__TASKPOOL__H__
