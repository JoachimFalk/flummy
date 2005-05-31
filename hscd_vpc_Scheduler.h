#include "systemc.h"
#ifndef HSCD_VPC_SCHEDULER_H
#define HSCD_VPC_SCHEDULER_H
#include "hscd_vpc_datatypes.h"
#include <map.h>

namespace SystemC_VPC{
  class Component;
  class Scheduler{
  public:
    virtual ~Scheduler() {};

    /**
     * /brief Called from SchedulerProxy to determine a "time slice" used as time out.
     * 
     */
    virtual int getSchedulerTimeSlice(sc_time &time,const map<int,p_struct> &ready_tasks,const map<int,p_struct> &running_tasks)=0;
    virtual void addedNewTask(int pid)=0;
    virtual void removedTask(int pid)=0;
    virtual scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign, map<int,p_struct> &ready_tasks, map<int,p_struct> &running_tasks)=0;

  };
}
#endif
