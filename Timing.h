#ifndef  __INCLUDED_TIMING_H__
#define __INCLUDED_TIMING_H__

#include <systemc.h>

#include "FastLink.h"

namespace SystemC_VPC{

  //helper struct
  struct Timing{
    sc_time dii;
    sc_time latency;
    FunctionId fid;
  };
  
}
#endif //__INCLUDED_TIMING_H__
