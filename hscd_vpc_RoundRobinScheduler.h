#ifndef HSCD_VPC_ROUNDROBINSCHEDULER_H
#define HSCD_VPC_ROUNDROBINSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <deque.h>

namespace SystemC_VPC{
  class Component;

  class RoundRobinScheduler : public Scheduler{
  public:

    RoundRobinScheduler(){
      TIMESLICE=1;
      LASTASSIGN=0;
    }
    RoundRobinScheduler(const char *schedulername);
    virtual ~RoundRobinScheduler(){}
    int getSchedulerTimeSlice(sc_time &time,const map<int,p_struct> &ready_tasks,const map<int,p_struct> &running_tasks);
    void addedNewTask(int pid);
    void removedTask(int pid);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign, map<int,p_struct> &ready_tasks, map<int,p_struct> &running_tasks);
    void setProperty(char* key, char* value);
  protected:
    deque<int> rr_fifo;
    double TIMESLICE;
    double LASTASSIGN;
  };
}
#endif
