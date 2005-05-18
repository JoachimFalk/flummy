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


enum action_command { assign,resign,retire,add};

typedef struct{
  int target_pid;
  action_command command;
}action_struct;


enum scheduling_decision {only_assign // neuer Task keine alten
			  ,preempt    // neuer Task verdrängt alten
			  ,resigned   // alter Task beendet, kein neuer
			  ,nochange}; //keine änderung 

#endif
