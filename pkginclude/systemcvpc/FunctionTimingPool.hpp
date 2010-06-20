#ifndef __INCLUDED__FUNCTION_TIMIG_POOL__H__
#define __INCLUDED__FUNCTION_TIMIG_POOL__H__

#include <map>
#include "FastLink.hpp"

namespace SystemC_VPC {
  class FunctionTiming;
  typedef std::map<ProcessId, FunctionTiming*>  FunctionTimingPool;
}
#endif // __INCLUDED__FUNCTION_TIMIG_POOL__H__
