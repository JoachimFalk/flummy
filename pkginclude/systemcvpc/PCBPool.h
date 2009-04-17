#ifndef __INCLUDED__PCBPOOL__H__
#define __INCLUDED__PCBPOOL__H__

#include <map>
#include "FastLink.h"

namespace SystemC_VPC {
  class ProcessControlBlock;
  typedef std::map<ProcessId, ProcessControlBlock*>  PCBPool;
}
#endif // __INCLUDED__PCBPOOL__H__
