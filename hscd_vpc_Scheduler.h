#include "systemc.h"
#ifndef HSCD_VPC_SCHEDULER_H
#define HSCD_VPC_SCHEDULER_H
#include "hscd_vpc_datatypes.h"
#include <map.h>


class Component;
class Scheduler{
 public:
  // virtual void schedule(int iprocess)=0;
/*
  virtual sc_event& getNotifyEvent()=0;  
  virtual void registerComponent(Component *comp)=0;
  virtual action_struct* getNextNewCommand(int pid)=0;
*/
  virtual ~Scheduler() {};
  virtual int getSchedulerTimeSlice(sc_time &time, map<int,p_struct> &ready_tasks, map<int,p_struct> &running_tasks)=0;
  virtual void addedNewTask(int pid)=0;
  virtual void removedTask(int pid)=0;
  virtual scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign, map<int,p_struct> &ready_tasks, map<int,p_struct> &running_tasks)=0;

};
#endif
