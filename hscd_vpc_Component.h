#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include "systemc.h"
#include "hscd_vpc_RateMonotonicScheduler.h"
#include "hscd_vpc_datatypes.h"
#include <vector.h>
#include <map.h>


class AbstractComponent{
 public:
  static char* NAMES[3];
  static const int RISC1=0;
  static const int RISC2=1;
  static const int RISC3=2;
  virtual void compute( const char *name )=0;
  virtual void compute(int iprocess)=0;
  virtual ~AbstractComponent(){};
};


class Component : public AbstractComponent{
 public:
  map<int,p_struct> &getNewTasks();
  vector<action_struct> &getNewCommands();
  sc_signal<bool> trace_signal[27];
  virtual void compute( const char *name );
  virtual void compute(int process);
  Component(int id);
  virtual ~Component();
  

 protected:
  map<int,p_struct>      new_tasks;
  vector<action_struct>  open_commands;
  sc_trace_file *trace;   ////////////////////////                              
  Scheduler *scheduler;
  int id;
  char* name;
  int mutex;
};
#endif
