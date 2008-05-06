#ifndef  __INCLUDED_TIMING_H__
#define __INCLUDED_TIMING_H__

#include <systemc.h>
namespace SystemC_VPC{

  //helper struct
  struct Timing{
    sc_time dii;
    sc_time latency;
    char*   fname;
  };
  
}
#endif //__INCLUDED_TIMING_H__
