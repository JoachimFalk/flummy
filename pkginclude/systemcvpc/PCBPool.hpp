#ifndef __INCLUDED__PCBPOOL__H__
#define __INCLUDED__PCBPOOL__H__

#include <map>
#include "FastLink.hpp"
#include "ProcessControlBlock.hpp"

namespace SystemC_VPC {
  typedef std::map<ProcessId, ProcessControlBlockPtr>  PCBPool;
}
#endif // __INCLUDED__PCBPOOL__H__
