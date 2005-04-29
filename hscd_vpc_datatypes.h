#include "systemc.h"
#ifndef HSCD_VPC_P_STRUCT_H
#define HSCD_VPC_P_STRUCT_H


typedef struct {
  char* name;
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


#endif
