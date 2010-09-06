#ifndef HSCD_VPC_P_STRUCT_H
#define HSCD_VPC_P_STRUCT_H
#include <systemc.h>

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
#define STR_PRIORITYSCHEDULERNOPREEMPT "PrioritySchedulerNoPreempt"
#define STR_PSNOPRE "PSNOPRE"
#define STR_RATEMONOTONIC "RateMonotonic"
#define STR_RM "RM"
#define STR_FIRSTCOMEFIRSTSERVE "FirstComeFirstServed"
#define STR_FCFS "FCFS"
#define STR_AVB "AVB"

// definition for hiding cosupport as vpc_event
typedef CoSupport::SystemC::RefCountEvent         VPC_Event;
  
// set for debugging output
//#define VPC_DEBUG true;

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
    bool operator()(const timePcbPair& left,
                    const timePcbPair& right) const
    {
      if (left.time > right.time)
        return true;
      else
        return false;
    }

  };

} // namespace SystemC_VPC
#endif