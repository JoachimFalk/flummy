#include "systemc.h"
#ifndef HSCD_VPC_P_STRUCT_H
#define HSCD_VPC_P_STRUCT_H
#define VPC_MAX_STRING_LENGTH 128

#include <string>
using std::string;


typedef struct {
  string name;
  int pid;
  sc_event* interupt;
  double delay;
  double remaining_delay;
  double priority;
  double period;
  double deadline;

}p_struct;


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
#define READY 'r';
#define RUNNING 'R';
//enum trace_value {blocked,ready,running};

#endif
