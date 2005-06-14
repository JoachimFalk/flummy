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
      if (pqe1.pcb.priority > pqe2.pcb.priority)
	return true;
      else if(pqe1.pcb.priority == pqe2.pcb.priority)
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


  enum scheduling_decision {ONLY_ASSIGN // neuer Task keine alten
			    ,PREEMPT    // neuer Task verdrängt alten
			    ,RESIGNED   // alter Task beendet, kein neuer
			    ,NOCHANGE}; //keine änderung 
  typedef char trace_value;

#define BLOCKED 'b';
#define READY   'w';
#define RUNNING 'R';
  //enum trace_value {blocked,ready,running};



} // namespace SystemC_VPC
#endif
