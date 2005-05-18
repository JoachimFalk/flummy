#ifndef HSCD_VPC_FCFSSCHEDULER_H
#define HSCD_VPC_FCFSSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <deque.h>


class Component;

class FCFSScheduler : public Scheduler{
 public:

  FCFSScheduler(){
      TIMESLICE=15;
  }
  virtual ~FCFSScheduler(){}
 protected:
  int getSchedulerTimeSlice(sc_time &time, map<int,p_struct> &ready_tasks, map<int,p_struct> &running_tasks);
  void addedNewTask(int pid);
  void removedTask(int pid);
  scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign, map<int,p_struct> &ready_tasks, map<int,p_struct> &running_tasks);
  deque<int> rr_fifo;
  double TIMESLICE;
};
#endif
