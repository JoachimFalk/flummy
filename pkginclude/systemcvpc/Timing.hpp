#ifndef  __INCLUDED_TIMING_H__
#define __INCLUDED_TIMING_H__

#include <systemc.h>

#include "FastLink.hpp"

namespace SystemC_VPC{

  //helper struct
  struct Timing{
    sc_time dii;
    sc_time latency;
    FunctionId fid;
    std::string functionName;
    std::string powerMode;
  };
  
}
#endif //__INCLUDED_TIMING_H__
