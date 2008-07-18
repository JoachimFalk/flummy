#ifndef HSCD_VPC_P_STRUCT_H
#define HSCD_VPC_P_STRUCT_H
#include <systemc.h>

// provide compatibility with other compilers then gcc, hopefully
//#include <ansidecl.h>

#include <CoSupport/SystemC/systemc_support.hpp>

namespace SystemC_VPC {
#define VPC_MAX_STRING_LENGTH 128
#define STR_TDMA "TDMA"
#define STR_FLEXRAY "FlexRay"
#define STR_TTCC "TTCC"
#define STR_ROUNDROBIN "RoundRobin"
#define STR_RR "RR"
#define STR_PRIORITYSCHEDULER "PriorityScheduler"
#define STR_PS "PS"
#define STR_RATEMONOTONIC "RateMonotonic"
#define STR_RM "RM"
#define STR_FIRSTCOMEFIRSTSERVE "FirstComeFirstServe"
#define STR_FCFS "FCFS"

// definition for hiding cosupport as vpc_event
typedef CoSupport::SystemC::Event         VPC_Event;
  
// set for debugging output
//#define VPC_DEBUG true;

#define STR_VPC_THREADEDCOMPONENTSTRING "threaded"
#define STR_VPC_DELAY "delay"
#define STR_VPC_LATENCY "latency"
#define STR_VPC_PRIORITY "priority"
#define STR_VPC_PERIOD "period"
#define STR_VPC_DEADLINE "deadline"

  class Task;

  struct p_queue_entry{
    int fifo_order;  // sekundärstrategie
    Task *task;
  };

  struct timePcbPair{
    sc_time time;
    Task *task;
  };

  struct timeCompare{
    bool operator()(const timePcbPair& tpp1,
        const timePcbPair& tpp2) const
    {
      sc_time p1=tpp1.time;
      sc_time p2=tpp2.time;
      if (p1 >= p2)
        return true;
      else
        return false;
    }

  };

} // namespace SystemC_VPC
#endif
