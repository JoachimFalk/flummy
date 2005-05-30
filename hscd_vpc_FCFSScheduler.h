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

  FCFSScheduler(const char *schedulername){
  }
  FCFSScheduler(){
  }
  virtual ~FCFSScheduler(){}
  int getSchedulerTimeSlice(sc_time &time,const map<int,p_struct> &ready_tasks,const map<int,p_struct> &running_tasks);
  void addedNewTask(int pid);
  void removedTask(int pid);
  scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign, map<int,p_struct> &ready_tasks, map<int,p_struct> &running_tasks);
 protected:

  deque<int> fcfs_fifo;
  //  double TIMESLICE;
};
#endif
