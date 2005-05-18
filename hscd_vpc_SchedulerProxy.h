#ifndef HSCD_VPC_SCHEDULERPROXY_H
#define HSCD_VPC_SCHEDULERPROXY_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <deque.h>


class Component;

class SchedulerProxy : public sc_module{
 public:
 
    SC_CTOR(SchedulerProxy){
    SC_THREAD(schedule_thread);
  }
  sc_event notify_scheduler;


  sc_event& getNotifyEvent();
  void registerComponent(Component *comp);
  action_struct* getNextNewCommand(int pid);

  void schedule_thread();

  virtual ~SchedulerProxy();
 protected:

  map<int,action_struct> *open_commands;
  map<int,p_struct> ready_tasks,running_tasks;
  Component *component;
  Scheduler *scheduler;

};
#endif
