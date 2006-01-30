#ifndef HSCD_VPC_P_STRUCT_H
#define HSCD_VPC_P_STRUCT_H
#include <systemc.h>

#include <string>
#include <map.h>

//#include <cosupport/systemc_support.hpp>
#include <systemc_support.hpp>

namespace SystemC_VPC {
#define VPC_MAX_STRING_LENGTH 128
#define STR_ROUNDROBIN "RoundRobin"
#define STR_RR "RR"
#define STR_PRIORITYSCHEDULER "PriorityScheduler"
#define STR_PS "PS"
#define STR_RATEMONOTONIC "RateMonotonic"
#define STR_RM "RM"
#define STR_FIRSTCOMEFIRSTSERVE "FirstComeFirstServe"
#define STR_FCFS "FCFS"

  /************************/
  /*  EXTENSION SECTION   */
  /************************/

#define STR_EARLIESTDEADLINEFIRST "EarliestDeadlineFirst"
#define STR_EDF "EDF"

// extension definition for tracing task state
#define S_SUSPENDED 's'
#define S_KILLED 'k'

// definitions for configuration file parsing
#define STR_VPC_RECONFIGURABLECOMPONENTSTRING "reconfigurable"

// definition for tracing configurations state
#define S_ACTIV 'A'
#define S_PASSIV 'P'
#define S_CONFIG 'c'

// definition for hiding cosupport as vpc_event
  typedef CoSupport::SystemC::Event VPC_Event;
  typedef CoSupport::SystemC::EventOrList VPC_EventOrList;
  
// set for debugging output
#define VPC_DEBUG true;
  
  /**************************/
  /*  END OF EXTENSION      */
  /**************************/

#define STR_VPC_MEASURE_FILE "measure.xml"
#define STR_VPC_RESULT_FILE "result"
#define STR_VPC_CONF_FILE "config"

#define STR_VPC_THREADEDCOMPONENTSTRING "threaded"
#define STR_VPC_COMPONENTSTRING "normal"
#define STR_VPC_DELAY "delay"
#define STR_VPC_PRIORITY "priority"
#define STR_VPC_PERIOD "period"
#define STR_VPC_DEADLINE "deadline"

#define RED(str) "\e[31;1m" <<str<< "\e[0m"
#define GREEN(str) "\e[32;1m" <<str<< "\e[0m"
#define YELLOW(str) "\e[33;1m" <<str<< "\e[0m" 
#define BLUE(str) "\e[34;1m" <<str<< "\e[0m"
#define WHITE(str) "\e[37;40m" <<str<< "\e[0m"
#define VPC_ERROR __FILE__<<":"<<__LINE__<<"\e[1;31;40mVPC: ERROR> " 
#define NORMAL "\e[0m"
#define NENDL "\e[0m"<<endl;
#ifdef MODES_EVALUATOR
#define  NO_VCD_TRACES
#endif // MODES_EVALUATOR
 

  using std::string;
  
  enum activation_state {inaktiv, 
               starting,
               aktiv,
               ending,
               aborted
               };

  typedef char trace_value;
#define S_BLOCKED 'b'
#define S_READY   'w'
#define S_RUNNING 'R'

  //enum trace_value {blocked,ready,running};


  struct p_struct {
    /* p_struct(const p_struct& p){
      cerr<< VPC_ERROR<< "copy constructor: p_struct(const p_struct& p)"<< NORMAL<<endl;
      pid=p.pid;
      name=p.name;
      interupt=p.interupt;
      delay=p.delay;
      remaining_delay =p.remaining_delay;
      priority=p.priority;
      period=p.period;
      deadline=p.deadline;
      activation_count=p.activation_count;
      state=p.state;
    }
    p_struct(){
      cerr<< VPC_ERROR<< "constructor: p_struct()"<< NORMAL<<endl;
      }*/
    string name;
    const char* funcname;
    int pid;
    sc_event* interupt;
    CoSupport::SystemC::Event* blockEvent;
    double delay;
    double remainingDelay;
    int priority;
    double period;
    double deadline;
    int activation_count;
    activation_state state;
    sc_signal<trace_value> *traceSignal;
    /*
     * Replaced by new version to enable multiple
     * delays for different components on different 
     * functions
      map<string,double>  functionDelays;
    */
    std::map<std::string, std::map<std::string, double> > compDelays;
    
  };


  struct p_queue_entry{
    int fifo_order;  // sekund?rstrategie
    p_struct *pcb;
  };
  
  struct p_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
        const p_queue_entry& pqe2) const
    {
      int p1=pqe1.pcb->priority;
      int p2=pqe2.pcb->priority;
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
      double p1=pqe1.pcb->priority/pqe1.pcb->period;
      double p2=pqe2.pcb->priority/pqe2.pcb->period;
      if (p1 > p2)
  return true;
      else if(p1 == p2)
  return (pqe1.fifo_order>pqe2.fifo_order);
      else 
  return false;
    }
    
  };



  enum action_command { ASSIGN,RESIGN,BLOCK,READY};

  typedef struct{
    int target_pid;
    action_command command;
  }action_struct;





} // namespace SystemC_VPC
#endif
