#ifndef HSCD_VPC_P_STRUCT_H
#define HSCD_VPC_P_STRUCT_H
#include "systemc.h"
#include <string>


namespace SystemC_VPC{
#define VPC_MAX_STRING_LENGTH 128

#define STR_ROUNDROBIN "RoundRobin"
#define STR_RR "RR"
#define STR_PRIORITYSCHEDULER "PriorityScheduler"
#define STR_PS "PS"
#define STR_RATEMONOTONIC "RateMonotonic"
#define STR_RM "RM"
#define STR_FIRSTCOMEFIRSTSERVE "FirstComeFirstServe"
#define STR_FCFS "FCFS"

  using std::string;


  typedef struct {
    string name;
    int pid;
    sc_event* interupt;
    double delay;
    double remaining_delay;
    int priority;
    double period;
    double deadline;

  }p_struct;


  struct p_queue_entry{
    int fifo_order;  // sekundärstrategie
    p_struct pcb;
  };
  struct p_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
		    const p_queue_entry& pqe2) const
    {
      int p1=pqe1.pcb.priority;
      int p2=pqe2.pcb.priority;
      if (p1 > p2)
	return true;
      else if(p1 == p2)
	return (pqe1.fifo_order>pqe2.fifo_order);
      else 
	return false;
    }
    
  };

  struct rm_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
		    const p_queue_entry& pqe2) const
    {
      double p1=pqe1.pcb.priority/pqe1.pcb.period;
      double p2=pqe2.pcb.priority/pqe2.pcb.period;
      if (p1 > p2)
	return true;
      else if(p1 == p2)
	return (pqe1.fifo_order>pqe2.fifo_order);
      else 
	return false;
    }
    
  };



  enum action_command { ASSIGN,RESIGN,RETIRE,ADD};

  typedef struct{
    int target_pid;
    action_command command;
  }action_struct;



  typedef char trace_value;

#define BLOCKED 'b';
#define READY   'w';
#define RUNNING 'R';
  //enum trace_value {blocked,ready,running};



} // namespace SystemC_VPC
#endif
