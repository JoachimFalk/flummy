#ifndef __INCLUDED__PCBPOOL__H__
#define __INCLUDED__PCBPOOL__H__

#include "Pool.h"
#include "FastLink.h"

namespace SystemC_VPC {
  class ProcessControlBlock;
  typedef AssociativePrototypedPool<ProcessId, ProcessControlBlock> PCBPool;

}
#endif // __INCLUDED__PCBPOOL__H__
