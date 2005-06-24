#ifndef HSCD_VPC_FCFSSCHEDULER_H
#define HSCD_VPC_FCFSSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <deque.h>
namespace SystemC_VPC{

  class Component;

  class FCFSScheduler : public Scheduler{
  public:

    FCFSScheduler(const char *schedulername){
    }
    FCFSScheduler(){
    }
    virtual ~FCFSScheduler(){}
    int getSchedulerTimeSlice(sc_time &time,const map<int,p_struct> &ready_tasks,const map<int,p_struct> &running_tasks);
    void addedNewTask(p_struct pcb);
    void removedTask(p_struct pcb);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  map<int,p_struct> &ready_tasks,const  map<int,p_struct> &running_tasks);
  protected:

    deque<int> fcfs_fifo;
    //  double TIMESLICE;
  };
}
#endif
